/* dzl-trie.c
 *
 * Copyright (C) 2012 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#include <glib/gi18n.h>

#include "search/dzl-trie.h"
#include "util/dzl-macros.h"

/**
 * SECTION:dzl-trie
 * @title: DzlTrie
 * @short_description: A generic prefix tree.
 *
 * The #DzlTrie struct and its associated functions provide a DzlTrie data structure,
 * where nodes in the tree can contain arbitrary data.
 *
 * To create a new #DzlTrie use dzl_trie_new(). You can free it with dzl_trie_free().
 * To insert a key and value pair into the #DzlTrie use dzl_trie_insert().
 * To remove a key from the #DzlTrie use dzl_trie_remove().
 * To traverse all children of the #DzlTrie from a given key use dzl_trie_traverse().
 */

typedef struct _DzlTrieNode      DzlTrieNode;
typedef struct _DzlTrieNodeChunk DzlTrieNodeChunk;

G_DEFINE_BOXED_TYPE (DzlTrie, dzl_trie, dzl_trie_ref, dzl_trie_unref)

/*
 * Try to optimize for 64 bit vs 32 bit pointers. We are assuming that the
 * 32 bit also has a 32 byte cacheline. This is very likely not the case
 * everwhere, such as x86_64 compiled with -m32. However, accomidating that
 * will require some changes to the layout of each chunk.
 */
#if GLIB_SIZEOF_VOID_P == 8
# define FIRST_CHUNK_KEYS         4
# define TRIE_NODE_SIZE          64
# define TRIE_NODE_CHUNK_SIZE    64
# define TRIE_NODE_CHUNK_KEYS(c) (((c)->is_inline) ? 4 : 6)
#else
# define FIRST_CHUNK_KEYS         3
# define TRIE_NODE_SIZE          32
# define TRIE_NODE_CHUNK_SIZE    32
# define TRIE_NODE_CHUNK_KEYS(c) (((c)->is_inline) ? 3 : 5)
#endif

/**
 * DzlTrieNodeChunk:
 * @flags: Flags describing behaviors of the DzlTrieNodeChunk.
 * @count: The number of items added to this chunk.
 * @keys: The keys for @children.
 * @next: The next #DzlTrieNodeChunk if there is one.
 * @children: The children #DzlTrieNodeChunk or %NULL. If the chunk is
 *   inline the DzlTrieNode, then there will be fewer items.
 */
#pragma pack(push, 1)
struct _DzlTrieNodeChunk
{
   DzlTrieNodeChunk *next;
   guint16           is_inline : 1;
   guint16           flags : 7;
   guint16           count : 8;
   guint8            keys[6];
   DzlTrieNode      *children[0];
};
#pragma pack(pop)

/**
 * DzlTrieNode:
 * @parent: The parent #DzlTrieNode. When a node is destroyed, it may need to
 *    walk up to the parent node and unlink itself.
 * @value: A pointer to the user provided value, or %NULL.
 * @chunk: The first chunk in the chain. Inline chunks have fewer children
 *    elements than extra allocated chunks so that they are cache aligned.
 */
#pragma pack(push, 1)
struct _DzlTrieNode
{
   DzlTrieNode      *parent;
   gpointer          value;
   DzlTrieNodeChunk  chunk;
};
#pragma pack(pop)

/**
 * DzlTrie:
 * @value_destroy: A #GDestroyNotify to free data pointers.
 * @root: The root DzlTrieNode.
 */
struct _DzlTrie
{
   volatile gint   ref_count;
   GDestroyNotify  value_destroy;
   DzlTrieNode    *root;
};

#if GLIB_SIZEOF_VOID_P == 8
  G_STATIC_ASSERT (sizeof(gpointer) == 8);
  G_STATIC_ASSERT ((FIRST_CHUNK_KEYS-1) == 3);
  G_STATIC_ASSERT (((FIRST_CHUNK_KEYS-1) * sizeof(gpointer)) == 24);
  G_STATIC_ASSERT(sizeof(DzlTrieNode) == 32);
  G_STATIC_ASSERT(sizeof(DzlTrieNodeChunk) == 16);
#else
  G_STATIC_ASSERT (sizeof(gpointer) == 4);
  G_STATIC_ASSERT ((FIRST_CHUNK_KEYS-1) == 2);
  G_STATIC_ASSERT (((FIRST_CHUNK_KEYS-1) * sizeof(gpointer)) == 8);
  G_STATIC_ASSERT(sizeof(DzlTrieNode) == 20);
  G_STATIC_ASSERT(sizeof(DzlTrieNodeChunk) == 12);
#endif

/**
 * dzl_trie_malloc0:
 * @trie: A #DzlTrie
 * @size: Number of bytes to allocate.
 *
 * Wrapper function to allocate a memory chunk. This gives us somewhere to
 * perform the abstraction if we want to use mmap()'d I/O at some point.
 * The memory will be zero'd before being returned.
 *
 * Returns: A pointer to the allocation.
 */
static gpointer
dzl_trie_malloc0 (DzlTrie *trie,
                  gsize    size)
{
   return g_malloc0(size);
}

/**
 * dzl_trie_free:
 * @trie: A #DzlTrie.
 * @data: The data to free.
 *
 * Frees a portion of memory allocated by @trie.
 */
static void
dzl_trie_free (DzlTrie  *trie,
               gpointer  data)
{
   g_free(data);
}

/**
 * dzl_trie_node_new:
 * @trie: A #DzlTrie.
 * @parent: The nodes parent or %NULL.
 *
 * Create a new node that can be placed in a DzlTrie. The node contains a chunk
 * embedded in it that may contain only 4 pointers instead of the full 6 do
 * to the overhead of the DzlTrieNode itself.
 *
 * Returns: A newly allocated DzlTrieNode that should be freed with g_free().
 */
DzlTrieNode *
dzl_trie_node_new (DzlTrie     *trie,
                   DzlTrieNode *parent)
{
   DzlTrieNode *node;

   node = dzl_trie_malloc0(trie, TRIE_NODE_SIZE);
   node->chunk.is_inline = TRUE;
   node->parent = parent;
   return node;
}

/**
 * dzl_trie_node_chunk_is_full:
 * @chunk: A #DzlTrieNodeChunk.
 *
 * Checks to see if all children slots are full in @chunk.
 *
 * Returns: %TRUE if there are no free slots in @chunk.
 */
static inline gboolean
dzl_trie_node_chunk_is_full (DzlTrieNodeChunk *chunk)
{
   g_assert(chunk);
   return (chunk->count == TRIE_NODE_CHUNK_KEYS(chunk));
}

/**
 * dzl_trie_node_chunk_new:
 * @trie: The DzlTrie that the chunk belongs to.
 *
 * Creates a new #DzlTrieNodeChunk with empty state.
 *
 * Returns: (transfer full): A #DzlTrieNodeChunk.
 */
DzlTrieNodeChunk *
dzl_trie_node_chunk_new (DzlTrie *trie)
{
   return dzl_trie_malloc0(trie, TRIE_NODE_CHUNK_SIZE);
}

/**
 * dzl_trie_append_to_node:
 * @chunk: A #DzlTrieNodeChunk.
 * @key: The key to append.
 * @child: A #DzlTrieNode to append.
 *
 * Appends @child to the chunk. If there is not room in the chunk,
 * then a new chunk will be added and append to @chunk.
 *
 * @chunk MUST be the last chunk in the chain (therefore chunk->next
 * is %NULL).
 */
static void
dzl_trie_append_to_node (DzlTrie          *trie,
                         DzlTrieNode      *node,
                         DzlTrieNodeChunk *chunk,
                         guint8            key,
                         DzlTrieNode      *child)
{
   g_assert(trie);
   g_assert(node);
   g_assert(chunk);
   g_assert(child);

   if (dzl_trie_node_chunk_is_full(chunk)) {
      chunk->next = dzl_trie_node_chunk_new(trie);
      chunk = chunk->next;
   }

   chunk->keys[chunk->count] = key;
   chunk->children[chunk->count] = child;
   chunk->count++;
}

/**
 * dzl_trie_node_move_to_front:
 * @node: A #DzlTrieNode.
 * @chunk: A #DzlTrieNodeChunk.
 * @idx: The index of the item to move.
 *
 * Moves the key and child found at @idx to the beginning of the first chunk
 * to achieve better cache locality.
 */
static void
dzl_trie_node_move_to_front (DzlTrieNode      *node,
                             DzlTrieNodeChunk *chunk,
                             guint             idx)
{
   DzlTrieNodeChunk *first;
   DzlTrieNode *child;
   guint8 offset;
   guint8 key;

   g_assert(node);
   g_assert(chunk);

   first = &node->chunk;

   key = chunk->keys[idx];
   child = chunk->children[idx];

   offset = ((first == chunk) ? first->count : FIRST_CHUNK_KEYS) - 1;
   chunk->keys[idx] = first->keys[offset];
   chunk->children[idx] = first->children[offset];

   memmove(&first->keys[1], &first->keys[0], (FIRST_CHUNK_KEYS-1));
   memmove(&first->children[1], &first->children[0], (FIRST_CHUNK_KEYS-1) * sizeof(DzlTrieNode *));

   first->keys[0] = key;
   first->children[0] = child;
}

/**
 * dzl_trie_find_node:
 * @trie: The #DzlTrie we are searching.
 * @node: A #DzlTrieNode.
 * @key: The key to find in this node.
 *
 * Searches the chunk chain of the current node for the key provided. If
 * found, the child node for that key is returned.
 *
 * Returns: (transfer none): A #DzlTrieNode or %NULL.
 */
static DzlTrieNode *
dzl_trie_find_node (DzlTrie     *trie,
                    DzlTrieNode *node,
                    guint8       key)
{
   DzlTrieNodeChunk *iter;
   guint i;

   g_assert(node);

   for (iter = &node->chunk; iter; iter = iter->next) {
      for (i = 0; i < iter->count; i++) {
         if (iter->keys[i] == key) {
            if (iter != &node->chunk) {
               dzl_trie_node_move_to_front(node, iter, i);
               __builtin_prefetch(node->chunk.children[0]);
               return node->chunk.children[0];
            }
            __builtin_prefetch(iter->children[i]);
            return iter->children[i];
         }
      }
   }

   return NULL;
}

/**
 * dzl_trie_find_or_create_node:
 * @trie: A #DzlTrie.
 * @node: A #DzlTrieNode.
 * @key: The key to insert.
 *
 * Attempts to find key within @node. If @key is not found, it is added to the
 * node. The child for the key is returned.
 *
 * Returns: (transfer none): The child #DzlTrieNode for @key.
 */
static DzlTrieNode *
dzl_trie_find_or_create_node (DzlTrie     *trie,
                              DzlTrieNode *node,
                              guint8       key)
{
   DzlTrieNodeChunk *iter;
   DzlTrieNodeChunk *last = NULL;
   guint i;

   g_assert(node);

   for (iter = &node->chunk; iter; iter = iter->next) {
      for (i = 0; i < iter->count; i++) {
         if (iter->keys[i] == key) {
            if (iter != &node->chunk) {
               dzl_trie_node_move_to_front(node, iter, i);
               __builtin_prefetch(node->chunk.children[0]);
               return node->chunk.children[0];
            }
            __builtin_prefetch(iter->children[i]);
            return iter->children[i];
         }
      }
      last = iter;
   }

   g_assert(last);

   node = dzl_trie_node_new(trie, node);
   dzl_trie_append_to_node(trie, node->parent, last, key, node);
   return node;
}

/**
 * dzl_trie_node_remove_fast:
 * @node: A #DzlTrieNode.
 * @chunk: A #DzlTrieNodeChunk.
 * @idx: The child within the chunk.
 *
 * Removes child at index @idx from the chunk. The last item in the
 * chain of chunks will be moved to the slot indicated by @idx.
 */
static inline void
dzl_trie_node_remove_fast (DzlTrieNode      *node,
                           DzlTrieNodeChunk *chunk,
                           guint             idx)
{
   DzlTrieNodeChunk *iter;

   g_assert(node);
   g_assert(chunk);

   for (iter = chunk; iter->next && iter->next->count; iter = iter->next) { }

   g_assert(iter->count);

   chunk->keys[idx] = iter->keys[iter->count-1];
   chunk->children[idx] = iter->children[iter->count-1];

   iter->count--;

   iter->keys[iter->count] = '\0';
   iter->children[iter->count] = NULL;
}

/**
 * dzl_trie_node_unlink:
 * @node: A #DzlTrieNode.
 *
 * Unlinks @node from the DzlTrie. The parent node has its pointer to @node
 * removed.
 */
static void
dzl_trie_node_unlink (DzlTrieNode *node)
{
   DzlTrieNodeChunk *iter;
   DzlTrieNode *parent;
   guint i;

   g_assert(node);

   if ((parent = node->parent)) {
      node->parent = NULL;
      for (iter = &parent->chunk; iter; iter = iter->next) {
         for (i = 0; i < iter->count; i++) {
            if (iter->children[i] == node) {
               dzl_trie_node_remove_fast(node, iter, i);
               g_assert(iter->children[i] != node);
               return;
            }
         }
      }
   }
}

/**
 * dzl_trie_destroy_node:
 * @trie: A #DzlTrie.
 * @node: A #DzlTrieNode.
 * @value_destroy: A #GDestroyNotify or %NULL.
 *
 * Removes @node from the #DzlTrie and releases all memory associated with it.
 * If the nodes value is set, @value_destroy will be called to release it.
 *
 * The reclaimation happens as such:
 *
 * 1) the node is unlinked from its parent.
 * 2) each of the children are destroyed, leaving us an empty chain.
 * 3) each of the allocated chain links are freed.
 * 4) the value pointer is freed.
 * 5) the structure itself is freed.
 */
static void
dzl_trie_destroy_node (DzlTrie        *trie,
                       DzlTrieNode    *node,
                       GDestroyNotify  value_destroy)
{
   DzlTrieNodeChunk *iter;
   DzlTrieNodeChunk *tmp;

   g_assert(node);

   dzl_trie_node_unlink(node);

   while (node->chunk.count) {
      dzl_trie_destroy_node(trie, node->chunk.children[0], value_destroy);
   }

   for (iter = node->chunk.next; iter;) {
      tmp = iter;
      iter = iter->next;
      dzl_trie_free(trie, tmp);
   }

   if (node->value && value_destroy) {
      value_destroy(node->value);
   }

   dzl_trie_free(trie, node);
}

/**
 * dzl_trie_new:
 * @value_destroy: A #GDestroyNotify, or %NULL.
 *
 * Creates a new #DzlTrie. When a value is removed from the trie, @value_destroy
 * will be called to allow you to release any resources.
 *
 * Returns: (transfer full): A newly allocated #DzlTrie that should be freed
 *   with dzl_trie_unref().
 */
DzlTrie *
dzl_trie_new (GDestroyNotify value_destroy)
{
   DzlTrie *trie;

   trie = g_new0(DzlTrie, 1);
   trie->ref_count = 1;
   trie->root = dzl_trie_node_new(trie, NULL);
   trie->value_destroy = value_destroy;

   return trie;
}

/**
 * dzl_trie_insert:
 * @trie: A #DzlTrie.
 * @key: The key to insert.
 * @value: The value to insert.
 *
 * Inserts @value into @trie located with @key.
 */
void
dzl_trie_insert (DzlTrie     *trie,
                 const gchar *key,
                 gpointer     value)
{
   DzlTrieNode *node;

   g_return_if_fail(trie);
   g_return_if_fail(key);
   g_return_if_fail(value);

   node = trie->root;

   while (*key) {
      node = dzl_trie_find_or_create_node(trie, node, *key);
      key++;
   }

   if (node->value && trie->value_destroy) {
      trie->value_destroy(node->value);
   }

   node->value = value;
}

/**
 * dzl_trie_lookup:
 * @trie: A #DzlTrie.
 * @key: The key to lookup.
 *
 * Looks up @key in @trie and returns the value associated.
 *
 * Returns: (transfer none): The value inserted or %NULL.
 */
gpointer
dzl_trie_lookup (DzlTrie     *trie,
                 const gchar *key)
{
   DzlTrieNode *node;

   __builtin_prefetch(trie);
   __builtin_prefetch(key);

   g_return_val_if_fail(trie, NULL);
   g_return_val_if_fail(key, NULL);

   node = trie->root;

   while (*key && node) {
      node = dzl_trie_find_node(trie, node, *key);
      key++;
   }

   return node ? node->value : NULL;
}

/**
 * dzl_trie_remove:
 * @trie: A #DzlTrie.
 * @key: The key to remove.
 *
 * Removes @key from @trie, possibly destroying the value associated with
 * the key.
 *
 * Returns: %TRUE if @key was found, otherwise %FALSE.
 */
gboolean
dzl_trie_remove (DzlTrie     *trie,
                 const gchar *key)
{
   DzlTrieNode *node;

   g_return_val_if_fail(trie, FALSE);
   g_return_val_if_fail(key, FALSE);

   node = trie->root;

   while (*key && node) {
      node = dzl_trie_find_node(trie, node, *key);
      key++;
   }

   if (node && node->value) {
      if (trie->value_destroy) {
         trie->value_destroy(node->value);
      }

      node->value = NULL;

      if (!node->chunk.count) {
         while (node->parent &&
                node->parent->parent &&
                !node->parent->value &&
                (node->parent->chunk.count == 1)) {
            node = node->parent;
         }
         dzl_trie_destroy_node(trie, node, trie->value_destroy);
      }

      return TRUE;
   }

   return FALSE;
}

/**
 * dzl_trie_traverse_node_pre_order:
 * @trie: A #DzlTrie.
 * @node: A #DzlTrieNode.
 * @str: The prefix for this node.
 * @flags: The flags for which nodes to callback.
 * @max_depth: the maximum depth to process.
 * @func: (scope call) (closure user_data): The func to execute for each matching node.
 * @user_data: User data for @func.
 *
 * Traverses node and all of its children according to the parameters
 * provided. @func is called for each matching node.
 *
 * This assumes that the order is %G_POST_ORDER, and therefore does not
 * have the conditionals to check pre-vs-pre ordering.
 *
 * Returns: %TRUE if traversal was cancelled; otherwise %FALSE.
 */
static gboolean
dzl_trie_traverse_node_pre_order (DzlTrie             *trie,
                                  DzlTrieNode         *node,
                                  GString             *str,
                                  GTraverseFlags       flags,
                                  gint                 max_depth,
                                  DzlTrieTraverseFunc  func,
                                  gpointer             user_data)
{
   DzlTrieNodeChunk *iter;
   gboolean ret = FALSE;
   guint i;

   g_assert(trie);
   g_assert(node);
   g_assert(str);

   if (max_depth) {
      if ((!node->value && (flags & G_TRAVERSE_NON_LEAVES)) ||
          (node->value && (flags & G_TRAVERSE_LEAVES))) {
         if (func(trie, str->str, node->value, user_data)) {
            return TRUE;
         }
      }
      for (iter = &node->chunk; iter; iter = iter->next) {
         for (i = 0; i < iter->count; i++) {
            g_string_append_c(str, iter->keys[i]);
            if (dzl_trie_traverse_node_pre_order(trie,
                                             iter->children[i],
                                             str,
                                             flags,
                                             max_depth - 1,
                                             func,
                                             user_data)) {
               return TRUE;
            }
            g_string_truncate(str, str->len - 1);
         }
      }
   }

   return ret;
}

/**
 * dzl_trie_traverse_node_post_order:
 * @trie: A #DzlTrie.
 * @node: A #DzlTrieNode.
 * @str: The prefix for this node.
 * @flags: The flags for which nodes to callback.
 * @max_depth: the maximum depth to process.
 * @func: (scope call) (closure user_data): The func to execute for each matching node.
 * @user_data: User data for @func.
 *
 * Traverses node and all of its children according to the parameters
 * provided. @func is called for each matching node.
 *
 * This assumes that the order is %G_POST_ORDER, and therefore does not
 * have the conditionals to check pre-vs-post ordering.
 *
 * Returns: %TRUE if traversal was cancelled; otherwise %FALSE.
 */
static gboolean
dzl_trie_traverse_node_post_order (DzlTrie             *trie,
                                   DzlTrieNode         *node,
                                   GString             *str,
                                   GTraverseFlags       flags,
                                   gint                 max_depth,
                                   DzlTrieTraverseFunc  func,
                                   gpointer             user_data)
{
   DzlTrieNodeChunk *iter;
   gboolean ret = FALSE;
   guint i;

   g_assert(trie);
   g_assert(node);
   g_assert(str);

   if (max_depth) {
      for (iter = &node->chunk; iter; iter = iter->next) {
         for (i = 0; i < iter->count; i++) {
            g_string_append_c(str, iter->keys[i]);
            if (dzl_trie_traverse_node_post_order(trie,
                                                  iter->children[i],
                                                  str,
                                                  flags,
                                                  max_depth - 1,
                                                  func,
                                                  user_data)) {
               return TRUE;
            }
            g_string_truncate(str, str->len - 1);
         }
      }
      if ((!node->value && (flags & G_TRAVERSE_NON_LEAVES)) ||
          (node->value && (flags & G_TRAVERSE_LEAVES))) {
         ret = func(trie, str->str, node->value, user_data);
      }
   }

   return ret;
}

/**
 * dzl_trie_traverse:
 * @trie: A #DzlTrie.
 * @key: The key to start traversal from.
 * @order: The order to traverse.
 * @flags: The flags for which nodes to callback.
 * @max_depth: the maximum depth to process.
 * @func: (scope call) (closure user_data): The func to execute for each matching node.
 * @user_data: User data for @func.
 *
 * Traverses all nodes of @trie according to the parameters. For each node
 * matching the traversal parameters, @func will be executed.
 *
 * Only %G_PRE_ORDER and %G_POST_ORDER are supported for @order.
 *
 * If @max_depth is less than zero, the entire tree will be traversed.
 * If max_depth is 1, then only the root will be traversed.
 */
void
dzl_trie_traverse (DzlTrie             *trie,
                   const gchar         *key,
                   GTraverseType        order,
                   GTraverseFlags       flags,
                   gint                 max_depth,
                   DzlTrieTraverseFunc  func,
                   gpointer             user_data)
{
   DzlTrieNode *node;
   GString *str;

   g_return_if_fail(trie);
   g_return_if_fail(func);

   node = trie->root;
   key = key ? key : "";

   str = g_string_new(key);

   while (*key && node) {
      node = dzl_trie_find_node(trie, node, *key);
      key++;
   }

   if (node) {
      if (order == G_PRE_ORDER) {
         dzl_trie_traverse_node_pre_order(trie, node, str, flags, max_depth, func, user_data);
      } else if (order == G_POST_ORDER) {
         dzl_trie_traverse_node_post_order(trie, node, str, flags, max_depth, func, user_data);
      } else {
         g_warning(_("Traversal order %u is not supported on DzlTrie."), order);
      }
   }

   g_string_free(str, TRUE);
}

/**
 * dzl_trie_unref:
 * @trie: A #DzlTrie or %NULL.
 *
 * Drops the reference count by one on @trie. When it reaches zero, the
 * structure is freed.
 */
void
dzl_trie_unref (DzlTrie *trie)
{
   g_return_if_fail(trie != NULL);
   g_return_if_fail(trie->ref_count > 0);

   if (g_atomic_int_dec_and_test(&trie->ref_count)) {
      dzl_trie_destroy_node(trie, trie->root, trie->value_destroy);
      trie->root = NULL;
      trie->value_destroy = NULL;
      g_free(trie);
   }
}

DzlTrie *
dzl_trie_ref (DzlTrie *trie)
{
  g_return_val_if_fail(trie != NULL, NULL);
  g_return_val_if_fail(trie->ref_count > 0, NULL);

  g_atomic_int_inc(&trie->ref_count);
  return trie;
}

/**
 * dzl_trie_destroy:
 * @trie: A #DzlTrie or %NULL.
 *
 * This is an alias for dzl_trie_unref().
 */
void
dzl_trie_destroy (DzlTrie *trie)
{
  if (trie != NULL)
    dzl_trie_unref (trie);
}

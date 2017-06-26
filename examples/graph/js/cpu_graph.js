#!/usr/bin/env gjs

imports.gi.versions["Gdk"] = "3.0"
imports.gi.versions["Gtk"] = "3.0"

const GLib = imports.gi.GLib;
const Gdk = imports.gi.Gdk;
const Gtk = imports.gi.Gtk;
const Dazzle = imports.gi.Dazzle;

Gtk.init(null);

/*
 * NOTE: If you are using Dazzle.Application as your base GApplication,
 *       then the following theme loading is handled for you. You need
 *       not manually register CSS.
 */
Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(),
                                         Dazzle.CssProvider.new ("resource:///org/gnome/dazzle/themes"),
                                         Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);

let win = new Gtk.Window({title: "Cpu Graph"});
let graph = new Dazzle.CpuGraph({timespan: GLib.TIME_SPAN_MINUTE, max_samples: 120});

win.add(graph);
win.connect('delete-event', function(){
  Gtk.main_quit();
});
win.show_all();

Gtk.main();

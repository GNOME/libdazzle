#ifndef EXAMPLE_APPLICATION_H
#define EXAMPLE_APPLICATION_H

#include <dazzle.h>

G_BEGIN_DECLS

#define EXAMPLE_TYPE_APPLICATION (example_application_get_type())

G_DECLARE_FINAL_TYPE (ExampleApplication, example_application, EXAMPLE, APPLICATION, DzlApplication)

G_END_DECLS

#endif /* EXAMPLE_APPLICATION_H */

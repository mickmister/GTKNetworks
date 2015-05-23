#include <gtk/gtk.h>

static GtkWidget *drawing_area;

void* startGUI();

static void drawWithoutBuffer(GtkWidget *widget, gdouble x, gdouble y, guint state, int colorIndex, unsigned int tempBrushSize);

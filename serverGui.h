#include <gtk/gtk.h>

GtkWidget *window, *drawing_area;

void* startGUI();

void drawWithoutBuffer(GtkWidget *widget, gdouble x, gdouble y, guint state, int colorIndex, unsigned int tempBrushSize);

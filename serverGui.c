
#include "serverGui.h"
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include	<unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "globals.h"


#define CHECK_SIZE 10
#define SPACING 2
#define BRUSH_SIZE_MIN 10
#define BRUSH_SIZE_MAX 20
#define DRAWING_AREA_SIZE 400
#define BUFFER_SIZE_MAX 100

GtkApplication *app;
static GtkWidget *window;
static cairo_surface_t *surface = NULL;

static void activate_drawing(GtkApplication* app, gpointer user_data);

static void clear_surface(GtkWidget *widget){
	cairo_t *cr;
	GtkAllocation alloc;
	
	gtk_widget_get_allocation(widget, &alloc);
	cr = cairo_create(surface);
	
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_paint(cr);
	cairo_destroy(cr);
}

static gboolean scribble_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data){
	GtkAllocation alloc;
	if(surface){
		cairo_surface_destroy(surface);
	}
	gtk_widget_get_allocation(widget, &alloc);
	surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget), 
																CAIRO_CONTENT_COLOR, 
																alloc.width, 
																alloc.height);
	clear_surface(widget);
	return TRUE;
}

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data){
	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_paint(cr);
	return FALSE;
}

static void drawWithoutBuffer(GtkWidget *widget, gdouble x, gdouble y, guint state, int colorIndex, unsigned int tempBrushSize)
{
	cairo_t *cr = NULL;
	
	if(x > DRAWING_AREA_SIZE || y > DRAWING_AREA_SIZE){
		return;
	}
	
	cr = cairo_create(surface);
	cairo_set_source_rgb(cr, colors[colorIndex][0], colors[colorIndex][1], colors[colorIndex][2]);
	cairo_rectangle(cr, x-(tempBrushSize/2), y-(tempBrushSize/2), tempBrushSize, tempBrushSize);
	
	cairo_fill(cr);
	cairo_destroy(cr);
	
	gtk_widget_queue_draw_area(widget, x-(tempBrushSize/2), y-(tempBrushSize/2), tempBrushSize, tempBrushSize);
}

static void close_window(GtkWidget *widget, gpointer data){
	if(surface){
		cairo_surface_destroy(surface);
	}gtk_widget_destroy(window);
	gtk_main_quit();
	
}

static void activate_drawing(GtkApplication* app, gpointer user_data){	
	gint size = 0;
	int width, height;
	
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Server Drawing");
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(close_window), NULL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_container_set_border_width(GTK_CONTAINER(window), 8);
	
	drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing_area, DRAWING_AREA_SIZE, DRAWING_AREA_SIZE);
	gtk_container_add(GTK_CONTAINER(window), drawing_area);
	g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_cb), NULL);
	g_signal_connect(G_OBJECT(drawing_area), "configure-event", G_CALLBACK(scribble_configure_event), NULL);
	gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK
								 | GDK_LEAVE_NOTIFY_MASK);
								 
	if(!gtk_widget_get_visible(window)){
		gtk_widget_show_all(window);
	}else{
		gtk_widget_destroy(window);
	}
}

void* startGUI(){
	char *array[] = {"server"};
	app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate_drawing), NULL);
	g_application_run(G_APPLICATION(app), 1, array);
	g_object_unref(app);	
	return;
}

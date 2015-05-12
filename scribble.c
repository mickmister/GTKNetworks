#include <gtk/gtk.h>
#include <string.h>

#define CHECK_SIZE 10
#define SPACING 2
#define BRUSH_SIZE_MIN 2
#define BRUSH_SIZE_MAX 20

static GtkWidget *window, *brushLabel, *entry, *drawing_area;
static cairo_surface_t *surface = NULL;
static brushSize = BRUSH_SIZE_MIN;
gdouble brush_color[] = {0, 0, 0};

static void change_color(GtkWidget *widget, gpointer data){
	char *color;
	color = gtk_entry_get_text(GTK_ENTRY(entry));
	sscanf(strtok(color, ",\0"), "%lf", &brush_color[0]);
	sscanf(strtok(NULL, ",\0"), "%lf", &brush_color[1]);
	sscanf(strtok(NULL, ",\0"), "%lf", &brush_color[2]);
}

static void clear_surface(GtkWidget *widget){
	cairo_t *cr;
	GdkPixbuf * pb;
	GtkAllocation alloc;
	GdkWindow *da_window = gtk_widget_get_window(widget);
	
	gtk_widget_get_allocation(widget, &alloc);
	cr = cairo_create(surface);
	
	if(gdk_window_is_viewable(da_window)){
		pb = gdk_pixbuf_get_from_window(da_window, 0, 0, alloc.width, alloc.height);
		gdk_pixbuf_save(pb, "file.bmp", "bmp", NULL, "quality", "100", NULL);
	}
	
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

static void draw_brush(GtkWidget *widget, gdouble x, gdouble y){
	cairo_t *cr = NULL;
	
	cr = cairo_create(surface);
	cairo_set_source_rgb(cr, brush_color[0], brush_color[1], brush_color[2]);
	cairo_rectangle(cr, x-(brushSize/2), y-(brushSize/2), brushSize, brushSize);
	
	cairo_fill(cr);
	cairo_destroy(cr);
	
	gtk_widget_queue_draw_area(widget, x-(brushSize/2), y-(brushSize/2), brushSize, brushSize);
}

static gint scribble_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data){
	if(surface == NULL){
		return FALSE;
	}
	
	if(event->button == GDK_BUTTON_PRIMARY){
		draw_brush(widget, event->x, event->y);
	}else if(event->button == GDK_BUTTON_SECONDARY){
		clear_surface(widget);
		gtk_widget_queue_draw(widget);
	}
	
	return TRUE;
}

static gboolean scribble_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data){	
	if(surface == NULL){
		return FALSE;
	}
	
	if(event->state & GDK_BUTTON1_MASK){
		draw_brush(widget, event->x, event->y);
	}
	
	return TRUE;
}

static gboolean scribble_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer data){
	char text[15];
	if(surface == NULL){
		return FALSE;
	}
	if(event->direction == GDK_SCROLL_UP){
		if(brushSize < BRUSH_SIZE_MAX){
			brushSize += 2;
		}
	}else if(event->direction == GDK_SCROLL_DOWN){
		if(brushSize > BRUSH_SIZE_MIN){
			brushSize -= 2;
		}
	}
	
	sprintf(text, "Brush Size: %d", brushSize);
	gtk_label_set_text(GTK_LABEL(brushLabel), text);
	
	return TRUE;
}

static void close_window(void){
	if(surface){
		cairo_surface_destroy(surface);
	}
	
	gtk_main_quit();
}

static void activate(GtkApplication* app, gpointer user_data){

	GtkWidget *button_color_change;
	GtkWidget *button_box;
	GtkWidget *image;
	GtkWidget *frame, *vbox, *label;
	
	GdkPixbuf *pixbuf;
	
	gint size = 0;
	int width, height;
	
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Drawing Area");
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(close_window), NULL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_container_set_border_width(GTK_CONTAINER(window), 8);
	
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b><u>Scribble area</u></b>");
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	
	brushLabel = gtk_label_new(NULL);
	gtk_label_set_text(GTK_LABEL(brushLabel), "Brush Size: 2");
	gtk_box_pack_start(GTK_BOX(vbox), brushLabel, FALSE, FALSE, 0);
	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
	
	button_color_change = gtk_button_new_with_label("Change Color");
	g_signal_connect(button_color_change, "clicked", G_CALLBACK(change_color), NULL);
	gtk_container_add(GTK_CONTAINER(vbox), button_color_change);
	
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	
	drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing_area, 400, 400);
	gtk_container_add(GTK_CONTAINER(frame), drawing_area);
	g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_cb), NULL);
	g_signal_connect(G_OBJECT(drawing_area), "configure-event", G_CALLBACK(scribble_configure_event), NULL);
	g_signal_connect(G_OBJECT(drawing_area), "motion-notify-event", G_CALLBACK(scribble_motion_notify_event), NULL);
	g_signal_connect(G_OBJECT(drawing_area), "button-press-event", G_CALLBACK(scribble_button_press_event), NULL);
	g_signal_connect(G_OBJECT(drawing_area), "scroll-event", G_CALLBACK(scribble_scroll_event), NULL);
	gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK
								 | GDK_LEAVE_NOTIFY_MASK
								 | GDK_BUTTON_PRESS_MASK
								 | GDK_POINTER_MOTION_MASK
								 | GDK_POINTER_MOTION_HINT_MASK
								 | GDK_SMOOTH_SCROLL_MASK);
								 
	

	/*
	pixbuf = gdk_pixbuf_new_from_file("image.jpg", NULL);
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	pixbuf = gdk_pixbuf_scale_simple(pixbuf, width / 4, height / 4, GDK_INTERP_BILINEAR);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_container_add(GTK_CONTAINER(vbox), image);
	//gdk_pixbuf_save(pixbuf, "file.bmp", "bmp", NULL, "quality", "100", NULL);
	*/
	if(!gtk_widget_get_visible(window)){
		gtk_widget_show_all(window);
	}else{
		gtk_widget_destroy(window);
	}
}

int main(int argc, char* argv[]){

	GtkApplication *app;
	int status;
	
	app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	
	
	return status;
}

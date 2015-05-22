#include <gtk/gtk.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "globals.h"

#define CHECK_SIZE 10
#define SPACING 2
#define BRUSH_SIZE_MIN 10
#define BRUSH_SIZE_MAX 20
#define DRAWING_AREA_SIZE 400
#define BUFFER_SIZE_MAX 300

int sockfd,n;
struct sockaddr_in servaddr,cliaddr;
char sendline[2000];
char recvline[2000];
clock_t start;

GtkApplication *app;
static GtkWidget *window, *brushLabel, *entry, *drawing_area, *image, *spinner;
static GtkWidget *window2, *entry_msg, *button_send, *box_msg;
GdkPixbuf * pb;
static cairo_surface_t *surface = NULL;
static brushSize = BRUSH_SIZE_MIN;
gdouble brush_color[] = {0, 0, 0};
int displayImage = 1, timer;
unsigned int bufferSize = 0;

COORDINATE_PAIR buffer[BUFFER_SIZE_MAX];

static void activate_connect(GtkApplication* app, gpointer user_data);
static void activate_drawing(GtkApplication* app, gpointer user_data);
static void draw_brush(GtkWidget *widget, gdouble x, gdouble y, guint state);
static void drawWithoutBuffer(GtkWidget *widget, gdouble x, gdouble y, guint state);

void *changeListener(void *socket_desc)
{
	PACKET packet;
	int i;
	COORDINATE_PAIR pair;
	int read_size;

	//Receive a message from client
    while( (read_size = recv(sockfd , &packet , sizeof(packet) , 0)) > 0 )
    {
    	for(i=0; i<packet.length; i++)
    	{
    		pair = packet.array[i];
    		printf("%3u  %3u  %3u\n", pair.x, pair.y, pair.brushSize);
    		draw_brush(drawing_area, pair.x, pair.y, 0);
    	}
    }
}

static void handle_buffer(COORDINATE_PAIR buffer[]){
	int i;
	int coordNum;
	printf("x    y    size\n---------------\n");
	int size;
	PACKET pack;
	for(i = 0; i < bufferSize; i+=PACKET_SIZE){
		if(bufferSize - i > PACKET_SIZE)
			size = PACKET_SIZE;
		else
			size = bufferSize - i;
		
		
		for(coordNum = 0; coordNum < size; coordNum++)
		{
			pack.array[coordNum] = buffer[i + coordNum];
		}
		
		pack.length = size;
		sendto(sockfd,&pack,sizeof(pack),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
   	// n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
   	// printf("%3u  %3u  %3u\n", buffer[i].x, buffer[i].y, buffer[i].brushSize);
	}
   recvline[n]=0;
}

static void handle_msg(char *msg){
	sprintf(sendline, "%s", gtk_entry_get_text(GTK_ENTRY(entry_msg)));
	sendto(sockfd,sendline,strlen(sendline),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
   n=recvfrom(sockfd,recvline,sizeof(recvline),0,NULL,NULL);
   printf("%s\n", recvline);
   recvline[n]=0;
}

static void connect_to_server(GtkWidget *widget, gpointer data, GtkApplication *app){
	gboolean active;
	INIT_PACKET init_pack;
	g_object_get(GTK_SPINNER(spinner), "active", &active, NULL);
	g_print("Connecting to %s\n", gtk_entry_get_text(GTK_ENTRY(entry)));
	gtk_spinner_start(GTK_SPINNER(spinner));
	servaddr.sin_addr.s_addr=inet_addr(gtk_entry_get_text(GTK_ENTRY(entry)));
	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0){
		activate_drawing(app, data);
		//n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
		//recvline[n]=0;
		n=recvfrom(sockfd,&init_pack,sizeof(init_pack),0,NULL,NULL);
      brush_color[0] = init_pack.r;
      brush_color[1] = init_pack.g;
      brush_color[2] = init_pack.b;
      printf("Color: r: %1.1f, g: %1.1f, b: %1.1f\n", brush_color[0], brush_color[1], brush_color[2]);
      //fputs(recvline,stdout);
      gtk_spinner_stop(GTK_SPINNER(spinner));

      pthread_t thread_id;
         
        if( pthread_create( &thread_id , NULL ,  changeListener , (void*) &sockfd) < 0)
        {
        	printf("Failed to create listening thread\n");
        }

	}
}

static void clear_surface(GtkWidget *widget){
	cairo_t *cr;
	GtkAllocation alloc;
	GdkWindow *da_window = gtk_widget_get_window(widget);
	
	gtk_widget_get_allocation(widget, &alloc);
	cr = cairo_create(surface);
	
	if(gdk_window_is_viewable(da_window)){
		pb = gdk_pixbuf_get_from_window(da_window, 0, 0, alloc.width, alloc.height);
		gdk_pixbuf_save(pb, "file.png", "png", NULL, NULL);
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
	if(displayImage){
		surface = cairo_image_surface_create_from_png("file.png");
		displayImage = 0;
	}
	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_paint(cr);
	return FALSE;
}

static void draw_brush(GtkWidget *widget, gdouble x, gdouble y, guint state)
{
	drawWithoutBuffer(widget, x, y, state);

	buffer[bufferSize].x = (unsigned int)x;
	buffer[bufferSize].y = (unsigned int)y;
	buffer[bufferSize].brushSize = (unsigned int)brushSize;
	bufferSize++;
	
	
	
}

static void drawWithoutBuffer(GtkWidget *widget, gdouble x, gdouble y, guint state)
{
	cairo_t *cr = NULL;
	
	if(x > DRAWING_AREA_SIZE || y > DRAWING_AREA_SIZE){
		return;
	}
	
	cr = cairo_create(surface);
	cairo_set_source_rgb(cr, brush_color[0], brush_color[1], brush_color[2]);
	cairo_rectangle(cr, x-(brushSize/2), y-(brushSize/2), brushSize, brushSize);
	
	cairo_fill(cr);
	cairo_destroy(cr);
	
	gtk_widget_queue_draw_area(widget, x-(brushSize/2), y-(brushSize/2), brushSize, brushSize);
	

}

static gint button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data){

	if(event->button == GDK_BUTTON_PRIMARY){
		handle_buffer(buffer);
		memset(buffer, 0, sizeof(buffer[0])*BUFFER_SIZE_MAX);
		bufferSize = 0;
	}
	return TRUE;
}

static gint scribble_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data){
	cairo_t *cr = NULL;
	if(surface == NULL){
		return FALSE;
	}
	displayImage = 0;
	if(event->button == GDK_BUTTON_PRIMARY){
		draw_brush(widget, event->x, event->y, event->state);
	}else if(event->button == GDK_BUTTON_SECONDARY){
		clear_surface(widget);
		gtk_widget_queue_draw(widget);
	}else{
		printf("middle-click\n");
	}
	
	return TRUE;
}

static gboolean scribble_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data){	
	if(surface == NULL){
		return FALSE;
	}
	
	if(event->state & GDK_BUTTON1_MASK){
		draw_brush(widget, event->x, event->y, event->state);
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

static void close_window(GtkWidget *widget, gpointer data){
	if(widget == window2){
		if(surface){
			cairo_surface_destroy(surface);
		}
		gtk_main_quit();
	}else{
		gtk_widget_destroy(window);
	}
}

static void activate_drawing(GtkApplication* app, gpointer user_data){

	GtkWidget *button_color_change;
	GtkWidget *button_box;
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
	gtk_label_set_text(GTK_LABEL(brushLabel), "Brush Size: 10");
	gtk_box_pack_start(GTK_BOX(vbox), brushLabel, FALSE, FALSE, 0);
	
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	
	drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing_area, DRAWING_AREA_SIZE, DRAWING_AREA_SIZE);
	gtk_container_add(GTK_CONTAINER(frame), drawing_area);
	g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_cb), NULL);
	g_signal_connect(G_OBJECT(drawing_area), "configure-event", G_CALLBACK(scribble_configure_event), NULL);
	g_signal_connect(G_OBJECT(drawing_area), "motion-notify-event", G_CALLBACK(scribble_motion_notify_event), NULL);
	g_signal_connect(G_OBJECT(drawing_area), "button-press-event", G_CALLBACK(scribble_button_press_event), NULL);
	g_signal_connect(G_OBJECT(drawing_area), "button-release-event", G_CALLBACK(button_release_event), NULL);
	g_signal_connect(G_OBJECT(drawing_area), "scroll-event", G_CALLBACK(scribble_scroll_event), NULL);
	gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK
								 | GDK_LEAVE_NOTIFY_MASK
								 | GDK_BUTTON_PRESS_MASK
								 | GDK_POINTER_MOTION_MASK
								 | GDK_POINTER_MOTION_HINT_MASK
								 | GDK_SMOOTH_SCROLL_MASK
								 | GDK_BUTTON_RELEASE_MASK);
								 
	if(!gtk_widget_get_visible(window)){
		gtk_widget_show_all(window);
	}else{
		gtk_widget_destroy(window);
	}
}

static void activate_connect(GtkApplication* app, gpointer user_data){
	GtkWidget *hbox_ip, *button_ip, *vbox;

	window2 = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window2), "Connection Window");
	g_signal_connect(G_OBJECT(window2), "destroy", G_CALLBACK(close_window), NULL);
	gtk_window_set_default_size(GTK_WINDOW(window2), 200, 100);
	gtk_container_set_border_width(GTK_CONTAINER(window2), 8);
	
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
	gtk_container_add(GTK_CONTAINER(window2), vbox);
	
	hbox_ip = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_container_set_border_width(GTK_CONTAINER(hbox_ip), 8);
	gtk_container_add(GTK_CONTAINER(vbox), hbox_ip);
	
	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox_ip), entry, FALSE, FALSE, 0);
	
	button_ip = gtk_button_new_with_label("Connect");
	g_signal_connect(button_ip, "clicked", G_CALLBACK(connect_to_server), app);
	gtk_container_add(GTK_CONTAINER(hbox_ip), button_ip);
	
	spinner = gtk_spinner_new();
	gtk_container_add(GTK_CONTAINER(hbox_ip), spinner);
	
	box_msg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_container_set_border_width(GTK_CONTAINER(box_msg), 8);
	gtk_container_add(GTK_CONTAINER(vbox), box_msg);
	
	entry_msg = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(box_msg), entry_msg, FALSE, FALSE, 0);
	
	button_send = gtk_button_new_with_label("Send");
	g_signal_connect(button_send, "clicked", G_CALLBACK(handle_msg), NULL);
	gtk_container_add(GTK_CONTAINER(box_msg), button_send);
	
	if(!gtk_widget_get_visible(window2)){
		gtk_widget_show_all(window2);
	}else{
		gtk_widget_destroy(window2);
	}
}

int main(int argc, char* argv[]){

	int status;
	
	sockfd=socket(AF_INET,SOCK_STREAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port=htons(8888);
	
	app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate_connect), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	
	
	return status;
}

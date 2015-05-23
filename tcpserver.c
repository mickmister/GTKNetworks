/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/
#include "serverGui.h"
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include "globals.h"

GdkPixbuf * pb;
int users[NUMUSERS];
 
//the thread function
void *connection_handler(void *);
void initialize();


void initialize()
{
    int i;
    for(i=0; i<NUMUSERS; i++)
    {
        users[i] = -1;
    }
}

int getIndex()
{
    int i;
    for(i=0; i<NUMUSERS; i++)
    {
        if(users[i] == -1)
        {
            return i;
        }
    }
    return -1;
}
 
int main(int argc , char *argv[])
{
    initialize();
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    pthread_t thread_id, gui_thread_id;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
    //*
    if(pthread_create(&gui_thread_id, NULL, startGUI, (void*)NULL) < 0){
    	printf("Failed to create GUI thread\n");
    }
    //*/
    //Listen
    listen(socket_desc , 3);
    
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	
	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}
 
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000]; 
    PACKET packet;
    FILE *fp;
    size_t result;
    int buff_size = 10240;
    char buff[buff_size];
    
    int userIndex = getIndex();
    if(userIndex == -1)
    {
        printf("User pool full, kicking user\n");
        return;
    }
    
   pb = gdk_pixbuf_get_from_window(GDK_WINDOW(window), 0, 0, DRAWING_AREA_SIZE, DRAWING_AREA_SIZE);
	gdk_pixbuf_save(pb, "file.png", "png", NULL, NULL);
	
	fp = fopen("file.png", "rb");
	while((result = fread(buff, 1, buff_size, fp)) > 0){
		send(sock, buff, result, 0);
	}
	fclose(fp);

    users[userIndex] = sock;

    INIT_PACKET initPack;
    initPack.colorIndex = userIndex;

    //Send color to client
    write(sock , &initPack , sizeof(initPack));
     
    //Receive a message from client
    while( (read_size = recv(sock , &packet , sizeof(packet) , 0)) > 0 )
    {
        if(packet.length > PACKET_SIZE)
        {
            printf("Error in packet size\n");
            continue;
        }
        
        int i;
        for(i=0; i<NUMUSERS; i++)
        {
            if(i != userIndex && users[i] != -1)
            {
                int sendingSocket = users[i];
                write(sendingSocket, &packet , sizeof(packet));
            }
        }

        int coordNum;
        COORDINATE_PAIR *array = packet.array;
        printf("PACKET size %d:\n", packet.length);
        for(coordNum = 0; coordNum < packet.length; coordNum++)
        {
            COORDINATE_PAIR pair = array[coordNum];
            drawWithoutBuffer(drawing_area, pair.x, pair.y, 0, packet.colorIndex, pair.brushSize);
        }
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
        users[userIndex] = -1;
    }
    else if(read_size == -1)
    {
        perror("recv failedyomans");
    }
         
    return 0;
}

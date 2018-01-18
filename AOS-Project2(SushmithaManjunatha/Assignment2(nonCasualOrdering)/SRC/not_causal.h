#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include <errno.h>
#include <iostream>

using namespace std;

int port_numb_array[3];
void *sender(void *);
void *listner(void *);
static int dat=6;
socklen_t client_length;     
struct hostent *server;
int no_of_arg=0;
char buffer[256];

int main(int argc, char *argv[])
{
     
    
    pthread_t send,recv;
    port_numb_array[1] = atoi(argv[1]);
    port_numb_array[2] = atoi(argv[2]);
    port_numb_array[3] = atoi(argv[3]);
// creating threads for receving and sending messages
    pthread_create(&recv,NULL,sender,NULL);
    pthread_create(&send,NULL,listner,NULL);
    pthread_join(send,NULL);
    pthread_join(recv,NULL);
        
}

void *sender(void *args){
    //Server side socket creation
    
    
    int sock_fd, new_sock_fd;
    socklen_t client_length;
    struct sockaddr_in serv_addr, client_address;
    int n,j;
    int number;
    char *d,*dp;

     while(1)
    {   //creating the socket
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) 
        cout<<"ERROR opening socket"<<endl;

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port_numb_array[1]);
		//error binding the socket
        if (bind(sock_fd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        cout<<"ERROR binding socket"<<endl;
	    //listening to socket
        int lis = listen(sock_fd,5);
        client_length = sizeof(client_address);
		//creating new socket for each process
        new_sock_fd = accept(sock_fd,(struct sockaddr *) &client_address,&client_length);
        if (new_sock_fd < 0) 
          cout<<"ERROR on accept"<<endl;
     // reading from buffer
        n = read(new_sock_fd,buffer,255);
        // seperating the values from buffer by using string tokenizer
        d = strtok(buffer,"\t");
        dp = strtok(NULL,"\n");
        cout<<endl;
		//printing the message receieved 
        cout<<"Message Recieved: "<<d<<"From portnumber"<<dp<<endl;
        cout<<endl;
        if (n < 0) 
        cout<<"ERROR reading from socket"<<endl;
        close(new_sock_fd);
        close(sock_fd); 
    }
    
    return 0; 
}


void *listner(void *args)
{
    
    int sock_fd,n,j;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    while(1){
        for(j=1;j<no_of_arg;j++)
        {   //creating the socket 
            sock_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (sock_fd < 0) 
            cout<<"ERROR opening socket"<<endl;
            server = gethostbyname("127.0.0.1");
            if (server == NULL)
            {
                fprintf(stderr,"ERROR, Host Not Found\n");
                exit(0);
            }
            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
            serv_addr.sin_port = htons(port_numb_array[j]);
            sleep(2);
            if (connect(sock_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
            cout<<"ERROR: Connection error"<<endl;
            //initilizing buffer to zeros
            bzero(buffer,256);
            sprintf(buffer,sizeof(buffer),"%d\t%d\n",dat,port_numb_array[j]);
            //printing the  buffer content
            cout<<"Buffer content: "<<buffer<<endl;
            dat++;
            cout<<"Sender Port: "<<port_numb_array[j]<<endl;
//writing to buffer
            n = write(sock_fd,buffer,strlen(buffer));
            if (n < 0) 
            cout<<"Write Error: Unable to write to socket"<<endl;

            close(sock_fd);//closing the socket 
        }
    }
    
    return 0;
}

#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <map>

#include <arpa/inet.h>
#include <time.h>
#include <string.h>

#include <netinet/in.h>
#include <netdb.h>

#include<iostream>
#include <stdio.h>
#include <sys/socket.h>
using namespace std;



void *handler_function(void * socket);


	 char buffer[1024];
         char backup[1024];
	//struct hostent *server_name;
        int socketfd1,socketfd2,socketfd3;  // 3 servers are being used

	int n;
	int NW=0;
	pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;  //mutexes are used for locking
       
	char server1_buffer[1024],server2_buffer[1024],server3_buffer[1024];
    int c_port, s1_port, s2_port, s3_port;
	int f = 0;

	int count=0;

int main(int argc, char *argv[])
{
	//struct sockaddr_in server_addr;
    
    struct hostent *server_name;
	int *new_socketfd;
        
	
	pthread_mutex_init(&mutex,NULL);

	if(argc<6)
	{
		cout<<"Usage frontendS <localhost> <client port> <server1 port> <server2 port> <server3 port>\n"<<endl;
		exit(1);
	}

	NW=0; //flag to keep track of number of not working servers initially it will be 0 assuming alls server works

	c_port=atoi(argv[2]);
	s1_port=atoi(argv[3]);
	s2_port=atoi(argv[4]);
	s3_port=atoi(argv[5]);

	int socketfd;
	socklen_t length;
    int reuse = 1;
    struct sockaddr_in serv_addr, another_addr;
    
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	return -1;

    //
    // allow the local host to reuse the port if the server is
    // terminated prematurely
    //
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse)) < 0) {
        cout<<"error in reuse address\n"<<endl;
    }
    
        
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(c_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if ((bind(socketfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) < 0) {
        cout<<"Error in binding the address"<<endl;
		close(socketfd);
		return -1;
    }

    length = sizeof(another_addr);
    
    if (getsockname(socketfd,(struct sockaddr*)&another_addr, (socklen_t*)&length) < 0) {
		cout<<"Error is getting socket name"<<endl;
        close(socketfd);
		return -1;
    }

    
    listen(socketfd, 5);
   
 	struct hostent *sh1,*sh2,*sh3; //for server1,2,3

	sh1= gethostbyname(argv[1]);
	sh2 = gethostbyname(argv[1]);
	sh3 = gethostbyname(argv[1]);

     //connect to the three servers
    struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	bcopy((char *)sh1->h_addr, (char *)&server_addr.sin_addr.s_addr,sh1->h_length);
	server_addr.sin_port = htons(s1_port);

	struct sockaddr_in server_addr1;

	server_addr1.sin_family = AF_INET;
	bcopy((char *)sh2->h_addr, (char *)&server_addr1.sin_addr.s_addr,sh2->h_length);
	server_addr1.sin_port = htons(s2_port);

	struct sockaddr_in server_addr2;

	server_addr2.sin_family = AF_INET;
	bcopy((char *)sh3->h_addr, (char *)&server_addr2.sin_addr.s_addr,sh3->h_length);
	server_addr2.sin_port = htons(s3_port);


	socketfd1 = socket(AF_INET, SOCK_STREAM, 0); //create socket for communication with the 
	if(socketfd1 < 0)
	{
		cout<<"Error creating the Socket\n"<<endl;
		exit(0);
	}

	if(connect(socketfd1, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		cout<<"Error connecting to server1\n"<<endl;
		NW=1;
	}

	socketfd2 = socket(AF_INET, SOCK_STREAM, 0); //create end point for communication
	if(socketfd2 < 0)
	{
		cout<< "Socket is not created"<<endl;
		exit(0);
	}

	if(connect(socketfd2, (struct sockaddr *) &server_addr1, sizeof(server_addr1)) < 0)
	{
		cout<<"Error connecting to server2\n"<<endl;
		NW=2;
	}

	socketfd3 = socket(AF_INET, SOCK_STREAM, 0); //create end point for communication
	if(socketfd3 < 0)
	{
		cout<<"Error creating the Socket \n"<<endl;
		exit(0);
	}

	if(connect(socketfd3, (struct sockaddr *) &server_addr2, sizeof(server_addr2)) < 0)
	{
		cout<<"Error connecting to server3\n"<<endl;
		NW=3;
	}


     //accepting requests from client
    struct sockaddr_in rem_addr;
    int nfd;
   

    length = sizeof(rem_addr);
    pthread_t t;

    while(nfd = accept(socketfd, (struct sockaddr*)&rem_addr, (socklen_t*)&length))
    {
        cout<<"NEW Connection accepted"<<endl;
        
	    new_socketfd = (int *)malloc(1);
	    *new_socketfd = nfd;

	    printf("Socket no %d\n", nfd);

	    bzero(buffer,1024);
	    strcpy(buffer,"OK");
	    //send OK to client
	    int s=write(nfd,buffer,1024);
	    if(s<0){
	    	cout<<"Error sending OK response to client\n"<<endl;
	    }
	    else{
	    	cout<<"OK message sent to client\n"<<endl;
	    }


	    if(pthread_create( &t , NULL ,  handler_function , (void*) new_socketfd) < 0)
	    {
	        cout<<"Error in  creation of the thread"<<endl;
	        return 1;
	    }
    }



	for(int index = 0; index < 100; index++)
	{
		pthread_join(t,NULL);
	}
	close(socketfd1);
	close(socketfd2);
	close(socketfd3);

	close(socketfd);
	exit(0);
}



//thread to send the messages to the sequencer as well as multicast the message to other processes
void *handler_function(void * socketfd)
{
	bzero(buffer, 1024);	
	cout<<"Accepting from client \n"<<endl;
	int newsocket = *(int *)socketfd;
	int l=0;
	//read multiple requests from clients
	while(1)
	{

		if(f==0)
		{
			n = read(newsocket, buffer, 1024);
			if(n < 0)
			{
				cout<<"Error reading from the client\n"<<endl;
			}
			else
			{
				if(strlen(buffer)!=0){
					cout<<"Read successful from client:"<< buffer<<endl;
				}
			}
				strcpy(backup,buffer);
			if(strlen(buffer)==0){
				if(l==10){			
					int j = write(newsocket,"OK\r\n",1024);
					if(j<0){
						fprintf(stderr, "Error\n");
					}
				}
				l++;
				pthread_mutex_unlock(&mutex);
			}
		}
		else
		{
			bzero(buffer,1024);
			strcpy(buffer,backup);
		}
				pthread_mutex_lock(&mutex);

		struct timeval timeout_values;     
    	timeout_values.tv_sec  = 0;
    	timeout_values.tv_usec = 1;

        int m= setsockopt (socketfd1, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout_values,sizeof(struct timeval)) ;
	if(m< 0)
        {
            cout<<"Error in setsockopt\n"<<endl;
        }

        

		//sending the command to server1
		if(NW!=1){
			int h=write(socketfd1, buffer, 1024);
			if(h < 0)
			{
				cout<<"Error writing to server1\n"<<endl;
			}
			else
			{
				cout<<"Write successful to server1:"<<buffer<<endl;
			}
		}

		//send command to server2
		if(NW!=2)
		{	  	
		  	int h=write(socketfd2, buffer, 1024);
			if(h < 0)
			{
				cout<<"Error writing to server2\n"<<endl;
			}
			else
			{
				cout<<"Write successful to server2\n"<<buffer<<endl;
			}
		}

		//send command to server3
		if(NW!=3){
			int h=write(socketfd3, buffer, 1024);
			if(h < 0)
			{
				cout<<"Error writing to server3\n"<<endl;
			}
			else
			{
				cout<<"Write successful to server3:" <<buffer<<endl;
			}
		}


        if (setsockopt (socketfd1, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout_values,sizeof(struct timeval)) < 0)
            {
            	cout<<"setsockopt failed\n"<<endl;
            }

		//READ OK FROM SERVERS
		//server1
		if(NW!=1)
		{
			bzero(server1_buffer,1024);
			int t=read(socketfd1, server1_buffer, 1024);
			if(t < 0)
			{
				cout<< "Error reading from server1\n"<<endl;
			}
			else
			{
				cout<<"Read successful from server1" << server1_buffer<<endl;
			}
		}

		//server2
		if(NW!=2)
		{
			bzero(server2_buffer,1024);
			int t=read(socketfd2, server2_buffer, 1024);
			if(t < 0)
			{
				cout<<"Error reading from server2\n"<<endl;
			}
			else
			{
				cout<<"Read successfully from server2: \n"<<server2_buffer<<endl;
			}
		}

		//server3
		if(NW!=3)
		{
			bzero(server3_buffer,1024);
			int t=read(socketfd3, server3_buffer, 1024);
			if(t < 0)
			{
				cout<<"Error reading from server3\n"<<endl;
			}
			else
			{
				cout<<"REad successful from server3\n"<<server3_buffer<<endl;
			}
		}
		
		usleep(200000);

		if(f==1 && (strlen(server1_buffer)!=0 || strlen(server2_buffer)!=0 || strlen(server3_buffer)!=0))
		{
				bzero(buffer, 1024);
				strcpy(buffer,"COMMIT");
				f=0;
		}

		//when OK has been received from all the servers
		//check if response has been received from all of them
		//if not, then think of maintaining data consistency...............
		else if(strlen(server1_buffer)!=0 && strlen(server2_buffer)!=0 && strlen(server3_buffer)!=0)

		{
			//if all servers are ready to commit
			if(strcmp(server1_buffer,"NO")!=0 && strcmp(server2_buffer,"NO")!=0 && strcmp(server3_buffer,"NO")!=0){
				//send global commit to all of them
				bzero(buffer, 1024);
				strcpy(buffer,"COMMIT");
				f=0;
			}
			else
			{
				//if even one of them sends a NO, ABORT the transaction and restart the transaction
				bzero(buffer,1024);
				strcpy(buffer,"ABORT");
				f=1;
			}
		}
		//if one of them do not reply/ one server crashes
		if(strlen(server1_buffer)==0){
			//didnt get response from socket 1
			NW=1;
			bzero(buffer, 1024);
			strcpy(buffer,"COMMIT");
			f=0;
		}
		if(strlen(server2_buffer)==0)
		{
			//dint get response from socket 2
			NW=2;
			bzero(buffer, 1024);
			strcpy(buffer,"COMMIT");
			f=0;
		}
		if(strlen(server3_buffer)==0){
			NW=3;
			bzero(buffer, 1024);
			strcpy(buffer,"COMMIT");
			f=0;
		}
		
		if(NW!=1)
		{
			int t=write(socketfd1, buffer, 1024);
			if(t < 0)
			{
				cout<< "Error writing to server1\n"<<endl;
			}
			else
			{
				cout<<"Write successful to server1\n"<<buffer<<endl;
			}
		}
		
		if(NW!=2)
		{
			int t=write(socketfd2, buffer, 1024);
			if(t < 0)
			{
				cout<< "Error writing to server2\n"<<endl;
			}
			else
			{
				cout<<"Write successful to server2 \n"<<buffer<<endl;
			}
		}

		if(NW!=3)
		{
			int t=write(socketfd3, buffer, 1024);
			if(t < 0)
			{
				cout<<"Error writing to server3\n"<<endl;
			}
			else
			{
				cout<<"Write successful to server3\n"<<buffer<<endl;
			}

		}

		usleep(200000);

		//READ OK FROM SERVERS
		//server1
		char final_value[1024];
		if(NW!=1){
			bzero(server1_buffer,1024);
			bzero(final_value,1024);
			int u=read(socketfd1, server1_buffer, 1024);
			if(u < 0)
			{
				cout<< "Error reading from server1\n"<<endl;
			}
			else
			{
				cout<<"Read successful from server1 %s\n"<<server1_buffer<<endl;
				strcpy(final_value,server1_buffer);
			}
			count++;
		}

		//server2
		if(NW!=2){
		bzero(server2_buffer,1024);
		bzero(final_value,1024);
		int t=read(socketfd2, server2_buffer, 1024);
		if(t < 0)
		{
			cout<<"Error reading from server2\n"<<endl;
		}
		else
		{
			printf("Read successful from server2 %s\n",server2_buffer);
			strcpy(final_value,server2_buffer);
		}
		count++;
	}
	
	//server3
	if(NW!=3){
		bzero(server3_buffer,1024);
		bzero(final_value,1024);
		int t=read(socketfd3, server3_buffer, 1024);
		if(t < 0)
		{
			cout<<"Error reading from server3\n"<<endl;
		}
		else
		{
			cout<<"REad successful from server3 \n"<<server3_buffer<<endl;
			strcpy(final_value,server3_buffer);
		}
		count++;
	}
		

		if(f==0 && count>0){
		//write the response to the client
			int t=write(newsocket,final_value,1024);
			if(t<0){
				cout<< "Error writing final result to client\n"<<endl;
			}
			else{
				cout<<"Written final result to client\n"<<endl;
			}
		}
				
		pthread_mutex_unlock(&mutex);
		bzero(buffer,1024);
	}
}



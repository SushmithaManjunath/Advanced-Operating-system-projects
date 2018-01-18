#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <cstdlib>	
#include <csignal>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
using namespace std;


#define NumOfProcesses 5

int main(int argc, char *argv[])
{
	
	int newsockfd[NumOfProcesses];
	int clocks[NumOfProcesses];
	
	int avg = 0;
	struct sockaddr_in serveradd, clientaddr;
	char buf[256];
        int  k = 0,l,m;
    //initial clock value
        int init_clock = rand() % 30 + 15;
        int socketfd, portnumb;

	if(argc < 2)
	{
		cout<<"Usage: server <port_number>\n"<<endl;
		exit(1);
	}

	cout <<"Daemon process's initial clockValue:"<< init_clock << endl;
        //creating the socket
	socketfd = socket(AF_INET,SOCK_STREAM, 0);
	if(socketfd < 0)
	  cout<<"stderr:socket creation failed\n"<<endl;
        
	bzero((char *)&serveradd, sizeof(serveradd));
        //initializing the address structure

	portnumb = atoi(argv[1]);
	serveradd.sin_family = AF_INET;
	serveradd.sin_addr.s_addr = INADDR_ANY;
	serveradd.sin_port = htons(portnumb);
	//binding the socket
	if(bind(socketfd, (struct sockaddr *)&serveradd, sizeof(serveradd)) < 0)
	  {
		cout<<"Error binding socket\n"<<endl;
	  }
         //listening at the port
	listen(socketfd, 5);

	socklen_t clientlen = sizeof(clientaddr);
	do{        //accepting new connections 
		newsockfd[k] = accept(socketfd, (struct sockaddr *)&clientaddr, &clientlen);
		if(newsockfd < 0)
		{
			cout<<"Error accepting client's sync request\n"<<endl;
		} 

		bzero(buf, 256);
		*(int *)buf = init_clock;
		int m=write(newsockfd[k],buf,256);
		if(m<0){
			cout<<"Error in sending daemon's initial clock\n"<<endl;
			exit(1);
		}

		bzero(buf, 256);
		m = read(newsockfd[k], buf, 256);
		if(m < 0)
		{
			fprintf(stderr, "Error in receiving client process's clock\n");
			exit(1);
		}
		cout <<"Difference in Clock value received"<<buf<< endl;
		//typecasting the buffer value to int		
		clocks[k] = *(int *)buf;
		k++;
	}while(k < NumOfProcesses);

	for(l = 0;l < NumOfProcesses;l++){
    //calculating the avg clock value of all the processes
		avg =  + clocks[l];
	}
        //calculating the avg
	avg = avg / (NumOfProcesses+1);
	cout <<"average calculated: "<< avg << endl;
        //updating the initial clock value
	init_clock = init_clock + avg;
	cout <<"Daemon Process's new clockValue " << init_clock << endl;
	bzero(buf,256);
	for (l = 0;l < NumOfProcesses;l++){
		bzero(buf,256);
		*(int *)buf=avg-clocks[l];
		cout<<"Writing to the buffer"<<buf <<endl;
                //writing the diff clock value to buffer
		int m = write(newsockfd[l], buf, 256);
		if(m < 0){
			fprintf(stderr, "Error writing synchronized clock to clients\n");
			close(newsockfd[l]);
			exit(1);
		}
	}
	
	for(l = 0;l < NumOfProcesses; l++)
		close(newsockfd[l]);
	
	close(socketfd);//closing the socket

	return 0;
}

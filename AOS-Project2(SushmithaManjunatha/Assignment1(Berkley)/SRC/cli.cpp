#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <strings.h>
#include <stdlib.h>
#include <pthread.h>
using namespace std;



int main(int argc, char *argv[])
{
          // client aruments check for errors
	if(argc < 4)
	{
		fprintf(stderr, "Usage client <hostname> <port number> <Clock_value> <ProcessID>\n");
		exit(1);
	}

	int portno,Clock_Value;
	portno = atoi(argv[2]);
	int pid=atoi(argv[4]);
	struct hostent *server;
	//convert hostname to networkbyte order
	server = gethostbyname(argv[1]);
	if(server == NULL)
	{
		fprintf(stderr, "Host doesnot exists\n");
		exit(1);
	}

	struct sockaddr_in server_addr;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,server->h_length);
	server_addr.sin_port = htons(portno);
	server_addr.sin_family = AF_INET;	

// Create a socket and check for errors during creation
	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd < 0)
	{
		fprintf(stderr, "Error creating the Socket \n");
		exit(0);
	}
//connecting to server
	if(connect(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		fprintf(stderr, "Error connecting to daemon process\n");
	}

	Clock_Value = atoi(argv[3]);
	char buf[256];
	bzero(buf,256);

	*(int *)buf = Clock_Value;
	cout<<"ProcessID:"<<pid<<"Initial Clock_Value of client:\n"<<Clock_Value<<endl;
//Reading Daemon's clock value
	int n=read(socketfd,buf,256);
	if(n<0){
		fprintf(stderr, "Error reading daemon process's initial clock\n");
		exit(1);
	}
	cout<<"ProcessID :"<<pid<<" Daemon's clock: "<< *(int *)buf <<endl;
//delta is the difference between Daemon's and initial clock value		
	int delta= Clock_Value-*(int *)buf;
	cout <<"processID"<<pid<<"Sending clock difference to daemon\n"<<delta<<endl;
	bzero(buf,256);
	*(int *)buf=delta;
	n = write(socketfd, buf, 256);
	if(n < 0)
	{
		fprintf(stderr, "%d : Error writing clock difference Value(delta) to daemon\n",pid);
		exit(1);	
	}
//Reading from the buffer the adjustment clock value from daemon process
	n = read(socketfd, buf, 256);
	if(n<0){
		fprintf(stderr, "Error reading clock time from daemon\n");
	}
	cout<<"ProcessID :"<<pid<<" Received difference \n"<<*(int *)buf<<endl;
	Clock_Value = Clock_Value+*(int *)buf;//clockvalue is updated(adjusted) accordingly
	cout<<"ProcessID:"<<pid<<"Syncronizing the Logical Clock :"<<Clock_Value<<endl;  	
	close(socketfd);
	exit(0);
}

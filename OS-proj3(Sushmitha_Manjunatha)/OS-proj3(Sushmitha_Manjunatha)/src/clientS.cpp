
#include <string.h>
#include <sys/socket.h>    
#include <arpa/inet.h> 
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
 using namespace std;
  

int main(int argc, char **argv) {
    if(argc < 3)   
	{
		cout<<"Usage client <hostname> <port number>"<<endl;
		exit(1);
	}

    
    struct hostent *server_name;
	int portnumber;
	portnumber = atoi(argv[2]);
	server_name = gethostbyname(argv[1]);
	if(server_name == NULL)
	{
		cout<<"Host name is wrong\n"<<endl;
		exit(1);
	}

	struct sockaddr_in server_address;

	server_address.sin_family = AF_INET;
	bcopy((char *)server_name->h_addr, (char *)&server_address.sin_addr.s_addr,server_name->h_length);
	server_address.sin_port = htons(portnumber);	

	int socketfd = socket(AF_INET, SOCK_STREAM, 0); //creating the socket
	if(socketfd < 0)
	{
		cout<<"Error forming the Socket \n"<<endl;
		exit(0);
	}

	if(connect(socketfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
	{
		cout<<"Error connecting to socket\n"<<endl;
	}

    char communication_buf[1024];
    int n=read(socketfd,communication_buf,1024); //reading ack from coordinator
    if(n<0){
    	cout<<"Error receiving OK from coordinator\n"<<endl;
    }
    else{
    	printf("%s\n", communication_buf);
    }
    int h;
    
    bzero(communication_buf,1024);// initilizing buffer to zero

	   	
    	do{
	   		printf("Enter the command\n");
	   		fgets(communication_buf,1024,stdin);// writing the command input taken by user to buffer 
	    	for(int k=0; k<strlen(communication_buf); k++){
		        if(islower(communication_buf[k])){ // converting the command to uppercase if the command is in lowercase 
		            communication_buf[k] = toupper(communication_buf[k]);
		        } else {
		            communication_buf[k] = communication_buf[k];
		        }
		    }
		    printf("%s\n", communication_buf);
	    	int v = write(socketfd, communication_buf, strlen(communication_buf));// writing to the coordinator
			if(v < 0)
			{	
				cout<<"Error writing to  the coordinator\n"<<endl;
				exit(1);	
			}
			bzero(communication_buf, 1024);
			v=read(socketfd,communication_buf,1024);// reading the final response from the coordinator 
			if(v<0)
			{
				cout<<"Error reading the final response from coordinator\n"<<endl;
			}
			else{
				printf("Received : %s\n", communication_buf);
				if(strcmp(communication_buf,"OK\r\n")==0)//checking whether it is a OK msg from coordinator to close the connection
				{
					cout<<"Connection is closed by the server.\n"<<endl;
					
					h=0;
					
				}
				scanf("%d",&h);
				bzero(communication_buf,1024);
			}
			
		}while(h!=0);
	
    close(socketfd);//closing the socket
	exit(0);
}

#include<cstdlib>	
#include<csignal>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
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
#include<ctime>

using namespace std;

#define MAXFILE 400
#define MAXCLIENTS 10
//#define CLOCKS_PER_SEC

int main(int argc,char *argv[])
{
	//clock_t begin=clock();

	if(argc < 5)
	{
		fprintf(stderr, "Usage client <hostname> <port number> <timestamp> <Transactions-file-path>\n");
		exit(1);
	}
	
	//socket connection
	struct hostent *server;
	server = gethostbyname(argv[1]);
	if(server == NULL)
	{
		fprintf(stderr, "No such host exists\n");
		exit(1);
	}

	int portno = atoi(argv[2]);
	
	float sec=atof(argv[3]);
	string filename=argv[4];
	
	struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,server->h_length);

	server_addr.sin_port = htons(portno);	


	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd < 0)
	{
		fprintf(stderr, "Socket not created \n");
		exit(0);
	}
	cout<<"Connecting to the server"<<endl;
	if(connect(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) // connecting to the server here
	{
		fprintf(stderr, "Error connecting to server\n");
	}
	
	//reading transaction.txt file by input stream
	ifstream input_stream(filename.c_str());  //input file stream is created to read file 

	int accno,timestamp;
	char operation;
	float amount;	
	int numofTrans = 0,n,i;
	char buff[256];	

	int initialtimestep=-1; // timestep is used for process synchronization

	
	      while(input_stream>>timestamp>>accno>>operation>>amount)    // reading each record from Transactions.txt file and sending it to the server for processing.
		{
		
		//input_stream>>timestamp>>accno>>operation>>amount;
		bzero(buff, 256);
		//getline(input_stream,buff);
		numofTrans++;
		sprintf (buff,"%d\t%d\t%c\t%f", timestamp,accno,operation,amount);	//these values will be stored in buffer
		cout<<timestamp<<accno<<operation<<amount<<endl;
		//double timeElapsed=(clock()-begin);
		//timeElapsed=(timeElapsed/(CLOCKS_PER_SEC/1000));
		double begin=clock();
		//cout << "Sending----->>buffer to server" << buff << endl;
		n = write(socketfd, buff, sizeof(buff));
		cout<<"value of buff:\t"<<buff<<endl;
		cout<<"Number of transactions happened so far:\t"<<numofTrans<<endl;
		if(n < 0)
		{
			fprintf(stderr, "Error with writing to socket\n");
			exit(1);	
		}
		
	

		if(initialtimestep!=-1)
		{
		cout<<"waiting to be served"<<endl;
		//timeElapsed=clock()-begin;
		//timeElapsed=timeElapsed/(CLOCKS_PER_SEC/1000);
		sleep((timestamp-initialtimestep)*sec);
		}
	
		initialtimestep=timestamp;
		bzero(buff, 256);
			n = read(socketfd, buff, sizeof(buff));
			double timetaken=clock()-begin;
			cout<<"time taken for the transaction by server\t"<<timetaken<<endl;
			cout<<"data receievd from server:"<<endl;
			cout<<buff;
			cout<<"finished reading from buffer"<<endl;		
			numofTrans++;
		
	}
	

	
	input_stream.close();     // closing input stream created to read file
     	
	
	close(socketfd);
	cout<<"Connection - END"<<endl;
	exit(0); 


	//return 0;
}
	

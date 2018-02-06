#include <iostream>
#include <fstream> 
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>

#define MAXTHREADS 100



using namespace std;

struct Transactions
{
	int timestamp;
	int accountno;
	char operation;
	float amount;
};


struct Records
{
	int accountno;
	string name;
	float amount;
}record[256];



/*** prototype declaration******/

void *ConditionHandler(void *);
void GetAccounts(Records record[], int numOfAcc);
void deposit(Records record[],Transactions, char*, int);
void withdrawal(Records record[],Transactions, char* ,int);

//declaring mutex 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc,char *argv[]) 
{
	

	if(argc < 3)
	{
		fprintf(stderr, "Usage: server <port_number> <RecordsFilename>\n");
		exit(1);
	}
	/**** Reading from the file Records.txt and storing in structures****/
	string filename = argv[2];//filename
	cout<<"input filename is\t"<<filename<<endl;

	ifstream input_stream(filename.c_str()); //input file stream is created
	
	/* The total number of accounts
	 Reading account numbers and balances from Text file*/
	
	int numOfAcc = 0;
	while(input_stream>>record[numOfAcc].accountno>>record[numOfAcc].name>>record[numOfAcc].amount)
        {
		numOfAcc++;

	} 
	
	pthread_t thread2; //declaring thread
	
	
	cout <<"number of accounts is:" << numOfAcc << endl;
	
	if(numOfAcc == -1) 
	{
		cout << "Error in reading the record" << endl;
		exit(1);
	} 
	else 
	{
       



        /**** Print initial database ****************/
	cout<<"AccountNo\t"<<"Name\t"<<"Amount\n";
	for(int i = 0; i<numOfAcc; i++) 
	{
		cout << record[i].accountno<<"\t"<<record[i].name<<"\t"<< record[i].amount << endl;
	}

	}




	/******* socket creation and initilization********/
	int socketfd, portno, threadNo;
	struct sockaddr_in serveradd, clientaddr;	
	
	
	socketfd = socket(AF_INET,SOCK_STREAM, 0);
	if(socketfd < 0)
	{	
		fprintf(stderr, "Error creating a socket\n");
	}
	 cout<<"Main socketfd value is\t:"<<socketfd<<endl; 
	bzero((char *)&serveradd, sizeof(serveradd));

	portno = atoi(argv[1]);
	serveradd.sin_family = AF_INET;
	serveradd.sin_addr.s_addr = INADDR_ANY;
	serveradd.sin_port = htons(portno);
	
	if(bind(socketfd, (struct sockaddr *)&serveradd, sizeof(serveradd)) < 0)
        {
		cout<<"Error binding socket"<<endl;
	}

	if(listen(socketfd, 200)!=0)
        {
		fprintf(stderr, "Error listening socket\n");
	}

			
	pthread_mutex_init(&mutex,NULL); // initilizing the mutex 



   /******* creating threads and socket for each client transactions ****/
			
	while(1)
	{
		socklen_t clientlen = sizeof(clientaddr);	
		int newsockfd = accept(socketfd, (struct sockaddr *)&clientaddr, &clientlen);
		cout<<"new socketfd is:\t"<<newsockfd<<endl;
		if(newsockfd < 0)
		{
			cout << "Error accepting client request\n" << endl;
		}
		
		// set thread detachstate attribute to DETACHED, so thread can be released after use 
		pthread_attr_t tattr; 
		pthread_attr_init(&tattr); //initilize thread attributes
		pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
		
		pthread_create(&thread2,&tattr,&ConditionHandler,&newsockfd);	//creating thread for each of the client transactions
		pthread_detach(thread2);
		threadNo++;
		
	 }
	
	close(socketfd);
	return 0;
} // closing main function 

/************ main function ends********************************/





void *ConditionHandler(void* args)
{

	int *tempfd= (int*) args;	
	int newsockfd= *tempfd;
			cout<<endl<<"func socketfd: "<<newsockfd<<endl;
	int n,timestamp, accno,accountIndex;
	float amount,depositAmount=0,previousBalance=0,withdrawalAmount=0;
	char op,buff[256],buff2[256];
	
	while(1)
	{
	Transactions trans;
//	pthread_mutex_init(&mutex,NULL); // initilizing the mutex 
	n = read(newsockfd, buff, sizeof(buff));
	cout<<"data recieved from the client"<<endl;
	if(n <= 0)
	{
		cout << "Exiting server thread";
		exit(1);
	}
	sscanf(buff,"%d %d %c %f",&timestamp,&accno,&op,&amount);
	trans.timestamp=timestamp;
	trans.accountno=accno;
	trans.operation=op;
	trans.amount=amount;
	
/*** Counting the actual number of accounts*******/
	Records rec_count[256];
	// Declare file object
	ifstream input_stream("Records.txt");
	int numOfAcc = 0;
	do
	{
		input_stream>>rec_count[numOfAcc].accountno>>rec_count[numOfAcc].name>>rec_count[numOfAcc].amount;
		numOfAcc++;
	}while(!input_stream.eof());


	cout<<"number of accounts present:\t"<<numOfAcc<<endl;
	


/****************performing operation  deposit or withdrawl or printing records************/ 
switch(trans.operation)
{
	case 'd':
	case 'D':
	                                     
		pthread_mutex_lock(&mutex);
		cout<<"***** entering critical section******"<<endl;
		deposit(record,trans,buff2,numOfAcc);
		pthread_mutex_unlock(&mutex);
		cout<<"***** lock released******"<<endl;
		break;
       case 'w':
       case 'W':
		cout<<"***** entering critical section******"<<endl;
	        pthread_mutex_lock(&mutex);
		withdrawal(record,trans,buff2,numOfAcc);
		pthread_mutex_unlock(&mutex);
		cout<<"******lock released******"<<endl;
		
		break;
	case 'g':
	case 'G':
		pthread_mutex_lock(&mutex);
		GetAccounts(record,numOfAcc);
		pthread_mutex_unlock(&mutex);
		break;
	

	default:

		cout<<endl<<"Invalid operation!";
		sprintf(buff2,"Invalid operation!");
	        break;
}//closing switch case

	n = write(newsockfd,buff2, sizeof(buff2));
	//cout<<"buff2 value\t:"<<buff2<<endl;
	if(n < 0)
	{	
		fprintf(stderr, "Error writing to socket\n");
	}

	
	

  }//closing while infine loop
		close(newsockfd);
}// ********************************************closing conditionhandler function




/*********************** Deposit function **************************/

void deposit(Records record[],Transactions trans,char* buff2,int numOfAcc)
{
	int n,timestamp, accno,accountIndex;
	float amount,depositAmount=0,previousBalance=0,withdrawalAmount=0;
	char op;
	// find that account in databases
	for(int index = 0; index < numOfAcc; index++) 
	{
		if(record[index].accountno == trans.accountno) 
		{
			
			accountIndex = index;
			cout <<"Account index:\t"<<index<<endl;

		}
		
	}
	
	//accountIndex = index;
	if(accountIndex != -1) 
	{
		if(trans.amount<0)
		{
			cout<<"Incorrect amount:"<<trans.amount<<", Deposit amount should be positive!"<<endl;
			sprintf(buff2,"\nIncorrect Amount: %.02f, Deposit amount should be positive! \n",trans.amount);	
		}
		else
		{
			//withdraw balance from account
			depositAmount=trans.amount;
			previousBalance = record[accountIndex].amount;
			record[accountIndex].amount += depositAmount;
			//Output: Previous and updated balance			
			sprintf(buff2,"\n%d %d %c %.02f\nPrevious Balance: $%.02f\nUpdated Balance %.02f\n",trans.timestamp,trans.accountno,trans.operation,trans.amount,previousBalance,record[accountIndex].amount);
			cout<<buff2;
			cout<<"account number:\t"<<trans.accountno<<"updated balance:\t"<<record[accountIndex].amount<<endl;
			cout<<"ACK:amount deposited successfully to account no\t:"<<trans.accountno<<endl;
			 
		}
        }
 
	else 
	{
		
		cout<<endl<<trans.timestamp<<" "<<trans.accountno<<" "<<trans.operation<<" "<<trans.amount<<" "<<endl;
		cout << "\nAccount number "<<trans.accountno<<" does not exist!\n";
		sprintf(buff2,"\nAccount number %d does not exist\n",trans.accountno);
	}
	
}





/********* withdraw function where withdraw operation takes place and balance is updated*************/
void withdrawal(Records record[],Transactions trans,char* buff2,int numOfAcc)
{
	int n,timestamp, accno,accountIndex=0;
	float amount,depositAmount=0,previousBalance=0,withdrawalAmount=0;
	char op;
	
	
	// find that account in databases
	for(int i = 0; i < numOfAcc; i++) 
	{
		if(record[i].accountno == trans.accountno) 
		{
			cout <<"Account index->\t "<<i<<endl;
			accountIndex = i;
			break;

		}
		

	}
	
	
	
	if(accountIndex != -1) 
	{	
		
		cout << record[accountIndex].amount;
		
		if(trans.amount>record[accountIndex].amount )
		{
			cout<<"insufficient balance in the account! "<<endl;	
			sprintf(buff2,"Balance insufficient!");	
		}
		else if(trans.amount<0)
		{
			cout<<"Incorrect amount:"<<trans.amount<<", withdrawal amount should be positive!"<<endl;
			sprintf(buff2,"\nIncorrect Amount: %.02f, withdrawal amount should be positive! \n",trans.amount);		
		}
		else
		{
			// balance to account

			
			withdrawalAmount=trans.amount;
			
			previousBalance = record[accountIndex].amount;
			
			record[accountIndex].amount -= withdrawalAmount;
			

			//Output: previous and updated balance
			cout <<"Acknowledgement Sent\n\n"<<endl;
			int n=sprintf(buff2,"\n%d %d %c %.02f\nPrevious Balance: $%.02f\nUpdated Balance %.02f\n",trans.timestamp,trans.accountno,trans.operation,trans.amount,previousBalance,record[accountIndex].amount);
			cout <<"value of buff2"<<buff2;
			cout <<"amount withdrawn sucessfully form account number:\t"<<trans.accountno<<endl; 
		}
	} 
	else 
	{
		
		cout <<endl<<trans.timestamp<<" "<<trans.accountno<<" "<<trans.operation<<" "<<trans.amount<<" "<<endl;
		cout << "\nAccount number "<<trans.accountno<<" does not exist!\n";
		sprintf(buff2,"\nAccount number %d does not exist\n",trans.accountno);
	}
	
	}


/***** this function to get the account information of a client ***************/


void GetAccounts(Records record[], int numOfAcc) 
{   	int ac=0;
	cout<<"enter account number of client who's information is requited\t:"<<endl;
	cin >> ac;
	for(int index = 0; index < numOfAcc; index++) 
	{
		if(record[index].accountno == ac) 
		{
			ofstream outputFile("Records1.txt");// Declare file object
			outputFile<<record[index].accountno<<" "<<record[index].name<<" "<<record[index].amount<<endl;
			cout<<"writing account information to file Record1.txt"<<endl;
	         }
			
			

        }
		
}
	
	
	
	

	
	









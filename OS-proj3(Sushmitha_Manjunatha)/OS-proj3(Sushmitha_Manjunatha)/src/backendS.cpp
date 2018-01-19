#include <sys/types.h>
#include <string.h>
#include <stdio.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include<iostream>
using namespace std;



pthread_mutex_t mutex1=PTHREAD_MUTEX_INITIALIZER; // creating mutexes for locking
pthread_mutex_t mutex2=PTHREAD_MUTEX_INITIALIZER;


typedef struct {
            int account_no; 
            float balance;
} records;

records R[100];

int main(int argc, char **argv)
{
    pthread_mutex_init(&mutex1,NULL);
        pthread_mutex_init(&mutex2,NULL);
    if(argc<3)
    {
      cout<< "Usage server <coordinator port> <server no>\n"<<endl;
        exit(1);
    }
    char msg_buffer[1024];
    int portnumber=atoi(argv[1]);
    int socketfd;
    socklen_t length;
    int serverno=atoi(argv[2]);
    int reuse = 1;
    FILE *myf;  
    struct sockaddr_in serv_addr, another_addr;
    
    int v = (socketfd = socket(AF_INET, SOCK_STREAM, 0));  //creating the socket
	if(v < 0)
    return -1;
    char file_name[20];
    sprintf(file_name, "Transaction_record%d.txt",serverno);   
   cout<<file_name<<endl;
    myf = fopen(file_name,"w"); //open the file with write permission
    if(myf==0){
      
	cout<<"error:"<<serverno<<" Unable to open the file"<<endl;
    }
    
    /** allows the port to reuse**/
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse)) < 0) 
    {
       cout<<serverno<<": SO_REUSEADDR error"<<endl;
    }
    
    /** setting adress attributes of socket**/   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portnumber);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    /** binding the socket**/
    int b = bind(socketfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
      if(b<0) {
      cout<<serverno<<" : cannot bind address"<<endl;
        close(socketfd);
        return -1;
    }

    length = sizeof(another_addr);
    
    if (getsockname(socketfd,(struct sockaddr*)&another_addr, (socklen_t*)&length) < 0) {
        cout<<serverno<<" : cannot get socket name"<<endl;
        close(socketfd);
        return -1;
    }

    
    listen(socketfd, 8); // listening to connect to the clients

 struct sockaddr_in o_addr;
        int new_fd;
       
        char trans[100];
        length = sizeof(o_addr);

        int socket_fd = accept(socketfd, (struct sockaddr*)&o_addr, (socklen_t*)&length);
        if(socket_fd<0)
        {
           cout<<serverno<<" : Error accepting socket\n"<<endl;
        } 

        int n;
        int i=0;
        int z=0;
        int accNumber=100;
        while(1)
        {
            n=read(socket_fd, msg_buffer, 1024);
            if(n<0){
               cout<<serverno<<": Error reading\n"<<endl;
            }
            if(strlen(msg_buffer)==0)
            {
                close(socket_fd);
                exit(1);
            }
            cout<<serverno<<": in server:"<< msg_buffer<<endl;

            //copy the transaction to "transaction" array
            strcpy(trans,msg_buffer);
            bzero(msg_buffer,1024);
            strcpy(msg_buffer,"YES");

         
               
            int x=write(socket_fd, msg_buffer,1024);
            if(x<0)
            {
                cout<< "Error writing yes/no to coordinator\n"<<endl;
            }

            //receive the commit msg
            bzero(msg_buffer,1024);
            int h=read(socket_fd,msg_buffer,1024);
            if(h<0){
               cout<<"Error reading commit/abort msg from coordinator\n"<<endl;
            }
            cout<<serverno<<" : commit:"<<msg_buffer<<endl;
           

            //check if it is global commit or abort
            if(strcmp(msg_buffer,"COMMIT")==0)
            {
                usleep(200000);
               cout<<"Going to commit\n"<<endl;
                

            //parse transaction and perform the task
            char *s;
            s=strtok(trans," ");
            char command[10];
            strcpy(command,s);
            if(strcmp(command,"QUIT\n")!=0)
            {
                s=strtok(NULL," ");
            }
            char line[200];
            int num_lines=0;
          
            if(strcmp(strdup(command),"CREATE")==0)
            {
                //create an account id
               
               cout<<"CREATE\n"<<endl;
              
                double amount=atof(s);
                
                if(amount<0)
                {
                    bzero(msg_buffer,1024);
                    sprintf(msg_buffer,"ERR creating the account (Amount is negative)\r\n");
                  cout<<"buffer:"<<msg_buffer<<endl;
                }
                //return the account no
                else{
                    R[i].balance=amount;
                    R[i].account_no=accNumber;
                    bzero(msg_buffer,1024);
                    sprintf(msg_buffer,"OK %d\r\n",R[i].account_no);
                    cout<<"buffer:"<<msg_buffer<<endl;
                    accNumber++;
                    i++;
                    fseek(myf, 0, SEEK_SET);
                    for(int k=0;k<i;k++){
                        fprintf(myf, "%d %.2f\n", R[k].account_no, R[k].balance);   
                    }
                }
                int t=write(socket_fd,msg_buffer, 1024);
                if(t<0){
                    cout<< serverno << ": Error writing to coordinator\n"<<endl;
                }
               
            }

            else if(strcmp(command,"QUERY")==0)
            {
                //query for checking account balance
                cout<<"QUERY\n"<<endl;
                int acc=atoi(s);
               
                int j;
                for(j=0;j<i;j++){
                    if(R[j].account_no==acc){
                       cout<< acc <<endl;
                        break;
                    }
                }
                //return the account no
                if(j<i && acc >=0){
                    bzero(msg_buffer,1024);
                    sprintf(msg_buffer,"OK %.2f\r\n",R[j].balance);
                    fseek(myf, 0, SEEK_SET);
                    for(int k=0;k<i;k++){
                    fprintf(myf, "%d %.2f\n", R[k].account_no, R[k].balance);   
                    }
                }
                else{
                    bzero(msg_buffer,1024);
                    sprintf(msg_buffer,"ERR Account %d does not exist\r\n",acc);
                }
                int a=write(socket_fd,msg_buffer, 1024);
                if(a<0){
                    cout<< serverno <<" : Error writing to coordinator\n"<<endl;
                }
            }

            else if(strcmp(command,"UPDATE")==0)
            {
                //update a record
                cout<<"UPDATE\n"<<endl;
                int acc=atoi(s);
                s=strtok(NULL," ");
                double amount=atof(s);
                int found=0;
                int j=0;
                for(j=0;j<i;j++){
                    if(R[j].account_no==acc){
                        pthread_mutex_lock(&mutex2);
                        found=1;
                        R[j].balance=amount;
                        pthread_mutex_unlock(&mutex2);
                        break;
                    }
                }
              
                //return the account no
                bzero(msg_buffer,1024);
                if(found==1)
                {
                sprintf(msg_buffer,"OK %.2f\r\n",R[j].balance);
                fseek(myf, 0, SEEK_SET);
                for(int k=0;k<i;k++){
                fprintf(myf, "%d %.2f\n", R[k].account_no, R[k].balance);   
                }
                }
                else
                {
                    sprintf(msg_buffer,"ERR Account %d does not exist\r\n",acc);
                }
                cout<<"Sending:"<< msg_buffer<<endl;
                int b=write(socket_fd,msg_buffer, 1024);
                if(b<0){
                    cout<<serverno<< ": Error writing to coordinator\n"<<endl;
                }
            }

            else if(strcmp(command,"QUIT\n")==0){
                //quit
                cout<<"QUIT\n"<<endl;
                bzero(msg_buffer,1024);
                strcpy(msg_buffer,"OK\r\n");
               cout<<"sending:"<<msg_buffer<<endl;
                int c=write(socket_fd, msg_buffer, 1024);
                if(c<0)
                {
                    cout<<"ERROR occured while sending ready to commit message to coordinator\n"<<endl;
                }
            }

            
        }
            else{
                //global abort, dont save data into file
                cout<<"Global abort sent\n"<<endl;
                bzero(msg_buffer,1024);
                strcpy(msg_buffer,"ABORT");
                int y=write(socket_fd, msg_buffer, 1024);
                if(y<0)
                {
                   cout<< "ERROR sending  abort  to coordinator\n"<<endl;
                }
            }
            
            bzero(msg_buffer,1024);
        }
        fflush(myf);
        fclose(myf);
        close(socket_fd);
        close(socketfd);
        
}

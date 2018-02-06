compile : 
	g++ -o server Bankserver.cpp -lm -lpthread
	g++ -o client newclient2C.cpp


runserver :
	./server 8888 Records.txt


runclient individually :
	./client.o 127.0.0.1 8888 0.4 Transactions.txt


runclient simultaneously :
  	chmod 755 script.sh
  	./script.sh

  
clean :
	rm server client
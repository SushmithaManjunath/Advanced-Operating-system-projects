compile :
	g++ backendS.cpp -o backendS -lpthread
	g++ frontendS.cpp -o frontendS -lpthread
	g++ clientS.cpp -o clientS


runBackendServer :
	./backendS 9090 1
	./backendS 9091 2
	./backendS 9092 3

runFrontendServer :
	./frontend localhost 8888 9090 9091 9092

runclient :
	./clientS localhost 8888


clean :
	rm backendS frontendS clientS

	

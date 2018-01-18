compile:
	g++ TimeDaemon.cpp -o DS.out -lpthread
	g++ cli.cpp -o cli.out
runserver:
	./DS.out 9090 3
runclient:
	chmod 755 clocks.sh
        ./clocks.sh
clean:
	rm *.out




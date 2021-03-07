
all: client server

server.o: server.cpp
	$(CXX) -Wall -c server.cpp -I.

client.o: client.cpp
	$(CXX) -Wall -c client.cpp -I.

main.o: main.cpp
	$(CXX) -Wall -c main.cpp -I.

client: client.o
	$(CXX) -L./ -Wall -o sspgame client.o

server: server.o
	$(CXX) -L./ -Wall -o sspd server.o

clean:
	rm *.o sspd sspgame

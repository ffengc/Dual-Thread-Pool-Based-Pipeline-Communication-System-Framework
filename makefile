.PHONY:all
all:client server

client:./Client/client.cc
	g++ -o $@ $^ -std=c++11;
	mv client Client
server:./Server/server.cc
	g++ -o $@ $^ -std=c++11
	mv server Server

.PHONY:clean
clean:
	rm -f ./Server/server ./Client/client
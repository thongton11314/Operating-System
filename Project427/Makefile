server: server.o
	./server 6507 10
server.o:
	g++ server.cpp -o server -lpthread


server2: server2.o
	./server2 6507 10

server2.o:
	g++ server2.cpp -o server2 -lpthread


client: client.o
	echo "10 * 15 runs"
	./client 6507 10 15 100 10.155.176.43 1

client.o:
	g++ client.cpp -o client


client2: client2.o
	echo "10 * 15 runs"
	./client2 6507 10 15 100 10.155.176.43 1

client2.o:
	g++ client2.cpp -o client2

clean:
	rm -f client server client2 server2
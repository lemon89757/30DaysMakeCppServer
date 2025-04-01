server:
	g++ server.cpp util.cpp -o server && \
	g++ client.cpp util.cpp -o client
clean:
	rm -f server client
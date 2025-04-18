server:
	g++ server.cpp \
	src/util.cpp src/Epoll.cpp src/InetAddress.cpp src/Socket.cpp src/Connection.cpp src/Buffer.cpp \
	src/Channel.cpp src/EventLoop.cpp src/Server.cpp src/Acceptor.cpp -o server && \
	g++ client.cpp src/util.cpp -o client
clean:
	rm -f server client
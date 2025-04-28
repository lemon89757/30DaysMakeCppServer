src=$(wildcard src/*.cpp)

# g++ -pthread(指定编译器在编译过程中需要链接 POSIX 线程库) 类似于 gcc -lpthread
# 但 -pthread 是一个综合性的选项，它同时处理编译和链接阶段的线程支持，-lpthread 只处理链接阶段，不涉及编译阶段
# 在实际开发中，推荐使用 -pthread，因为它更简单、更安全，能够确保程序在编译和运行时都能正确地使用线程功能

server:
	g++ -Wall -std=c++11 -pthread -g \
	$(src) \
	server.cpp \
	-o server
client:
	g++ -Wall src/util.cpp src/Buffer.cpp src/Socket.cpp src/InetAddress.cpp client.cpp -o client

th:
	g++ -Wall -pthread src/ThreadPool.cpp ThreadPoolTest.cpp -o ThreadPoolTest

test:
	g++ -Wall src/util.cpp src/Buffer.cpp src/Socket.cpp src/InetAddress.cpp src/ThreadPool.cpp \
	-pthread \
	test.cpp -o test

clean:
	rm -f server client test
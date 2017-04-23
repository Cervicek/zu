all: ipk-client.cpp
	g++ -std=c++11 -o ipk-client  ipk-client.cpp -lcrypto -lssl

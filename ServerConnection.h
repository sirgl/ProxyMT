#ifndef PROXYMT_SERVERCONNECTION_H
#define PROXYMT_SERVERCONNECTION_H


#include <iostream>
#include "Proxy.h"
#include "ClientConnection.h"
#include "ProxyException.h"

static const char *const OK_ANSWER = "200";

class ServerConnection {
private:
	Proxy &proxy;
	ClientConnection &clientConnection;
	int serverSocket;

	char buffer[BUFFER_CAPACITY];
	int bufferSize;
public:

	ServerConnection(Proxy &proxy, ClientConnection &connection);

	~ServerConnection();

	void start();

	void connectToHost();

	void notifyClientStateChanged(ServerThreadState state, TransmissionState transmissionState);

	void sendRequestToServer();

	void receiveResponseFromServer();

	bool isOkResponse();

	void startNonCachingTransmission();

	void copyDataToClientBuffer();

	void receiveDataFromServer();

	void startCachingTransmission();

	void copyDataToCache();
};

struct ServerConnectionThreadArguments {
	Proxy &proxy;
	ClientConnection &connection;

	ServerConnectionThreadArguments(Proxy &proxy, ClientConnection &connection) :
			proxy(proxy),
			connection(connection) { }
};

void * serverConnectionThreadHelper(ServerConnectionThreadArguments* arguments);;


#endif //PROXYMT_SERVERCONNECTION_H

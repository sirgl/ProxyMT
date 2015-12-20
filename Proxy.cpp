#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <netdb.h>
#include <string.h>

#include "Proxy.h"
#include "ProxyException.h"
#include "tools.h"
#include "SocketException.h"
#include "ClientConnection.h"


Proxy::Proxy(char *listenPort) {
	struct addrinfo hint;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_PASSIVE;
	int status;
	struct addrinfo *listenInfo;
	if ((status = getaddrinfo(NULL, listenPort, &hint, &listenInfo)) != 0) {
		throw ProxyException("getaddrinfo: " + string(gai_strerror(status)));
	}

	proxySocket = socket(listenInfo->ai_family, listenInfo->ai_socktype, listenInfo->ai_protocol);
	if (-1 == proxySocket) {
		throw SocketException("Can't create clientSocket for proxy " + getError(errno));
	}

	int optval = 1;
	if (-1 == setsockopt(proxySocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))) {
		throw SocketException("Can't set reuse address: " + getError(errno));
	}
	if (-1 == bind(proxySocket, listenInfo->ai_addr, listenInfo->ai_addrlen)) {
		throw SocketException("Failed to bind proxy to port: " + getError(errno));
	}
	if (-1 == (listen(proxySocket, SOMAXCONN))) {
		throw SocketException("Failed to listen to port: " + getError(errno));
	}
	std::cout << "Proxy started on " << listenPort << std::endl;
}

void Proxy::start() {
	while (true) {
		struct sockaddr_in clientAddress;
		socklen_t length = sizeof(clientAddress);
		int acceptedSocket = accept(proxySocket, (sockaddr *) (&clientAddress), &length);
		if (-1 == acceptedSocket) {
			throw IOException("Failed to accept client: " + getError(errno));
		}
		std::cout << "Connection accepted" << std::endl;

		createClientConnectionThread(acceptedSocket);
	}
}

void Proxy::createClientConnectionThread(int acceptedSocket) {
	pthread_t thread;
	auto arguments = new ClientConnectionThreadArguments(*this, acceptedSocket);
	auto error = pthread_create(&thread, nullptr, (void *(*)(void *)) clientConnectionHepler, arguments);
	if (error) {
		throw ProxyException("Can't create thread: " + getError(errno));
	}
	pthread_detach(thread);
}

Proxy::~Proxy() {
	close(proxySocket);
	std::cout << "Proxy closed" << std::endl;
}

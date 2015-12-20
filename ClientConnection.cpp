#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <assert.h>
#include "ClientConnection.h"
#include "ProxyException.h"
#include "tools.h"
#include "IOException.h"
#include "parser/RequestParser.h"
#include "ServerConnection.h"

void *clientConnectionHepler(ClientConnectionThreadArguments *arguments) {
	ClientConnection *connection = new ClientConnection(arguments->socket, arguments->proxy);
	try {
		connection->start();
	} catch (ProxyException &exception) {
		std::cerr << exception.what() << std::endl;
	}
	delete (arguments);

	return nullptr;
}

ClientConnection::ClientConnection(int socket, Proxy &proxy) :
		clientSocket(socket),
		proxy(proxy),
		bufferSize(0) {
	transmissionType = TransmissionType::NO_TRANSMISSION;
	transmissionState = TransmissionState::LOADING;
	serverThreadState = ServerThreadState::NO_THREAD;
	pthread_mutex_init(&serverAnswerMutex, nullptr);
	pthread_cond_init(&serverThreadAnswered, nullptr);
	pthread_mutex_init(&bufferMutex, nullptr);
	pthread_cond_init(&bufferAvailable, nullptr);
	std::cout << "Client connection " << clientSocket << " created" << std::endl;
}

ClientConnection::~ClientConnection() {
	pthread_mutex_destroy(&bufferMutex);
	pthread_mutex_destroy(&serverAnswerMutex);
	pthread_cond_destroy(&bufferAvailable);
	pthread_cond_destroy(&serverThreadAnswered);
	close(clientSocket);
	std::cout << "Client connection " << clientSocket << " removed" << std::endl;
}

void ClientConnection::start() {
	receiveRequest();
	parseRequest();

	if (proxy.cacheStorage.contains(url)) {
		startCaching();
	} else {
		createServerConnectionThread();
		waitForServerConnection();
		if (transmissionState == TransmissionState::ERROR) {
			throw IOException("Server thread error");
		}
		if (TransmissionType::NO_TRANSMISSION == transmissionType) {
			assert(true);
		} else if (TransmissionType::CACHING == transmissionType) {
			startCaching();
		} else if (TransmissionType::NON_CACHING == transmissionType) {
			startNonCaching();
		}

	}
}

void ClientConnection::receiveRequest() {
	bufferSize = (int) read(clientSocket, buffer, BUFFER_CAPACITY);
	if (-1 == bufferSize) {
		throw IOException("Can't receive request from clientSocket: " + getError(errno));
	}
}

void ClientConnection::parseRequest() {
	RequestParser parser(string(buffer, (unsigned long) bufferSize));

	Request *request;
	try {
		request = parser.parse();
	} catch (const ParserException &e) {
		write(clientSocket, BAD_REQUEST, sizeof(BAD_REQUEST));
		throw ProxyException("Bad request");

	}
	request->print();
	if ("GET" != request->method) {
		write(clientSocket, METHOD_NOT_SUPPORTED, sizeof(METHOD_NOT_SUPPORTED));
		throw ProxyException("Method not supported");
	}
	if ("HTTP/1.0" != request->protocol) {
		write(clientSocket, HTTP_VERSION_NOT_SUPPORTED, sizeof(HTTP_VERSION_NOT_SUPPORTED));
		throw ProxyException("Http version not supported");
	}
	url = request->url;
	host = request->getHost();
}

void ClientConnection::createServerConnectionThread() {
	pthread_t thread;
	auto arguments = new ServerConnectionThreadArguments(proxy, *this);
	auto error = pthread_create(&thread, nullptr, (void *(*)(void *)) serverConnectionThreadHelper, arguments);
	if (error) {
		throw ProxyException("Can't create thread: " + getError(errno));
	}
	pthread_detach(thread);
}

void ClientConnection::waitForServerConnection() {
	pthread_mutex_lock(&serverAnswerMutex);
	while (ServerThreadState::NO_THREAD == serverThreadState) {
		pthread_cond_wait(&serverThreadAnswered, &serverAnswerMutex);
	}
	pthread_mutex_unlock(&serverAnswerMutex);
}

void ClientConnection::startNonCaching() {
	sendDataToClient();
}

void ClientConnection::sendDataToClient() {
	pthread_mutex_lock(&bufferMutex);
	while (true) {
		if (bufferSize == 0) {
			pthread_cond_wait(&bufferAvailable, &bufferMutex);
			if (transmissionState == TransmissionState::ERROR) {
				pthread_cond_signal(&bufferAvailable);
				pthread_mutex_unlock(&bufferMutex);
				throw IOException("Error in server thread " + getError(errno));
			}
			continue;
		}
		auto bytesWrite = write(clientSocket, buffer, (size_t) bufferSize);
		if (-1 == bytesWrite) {
			pthread_cond_signal(&bufferAvailable);
			pthread_mutex_unlock(&bufferMutex);
			throw IOException("Error writing to client: " + getError(errno));
		}
		std::cout << "Data send to client" << std::endl;
		bufferSize = 0;
		if (TransmissionState::LOADING == transmissionState) {
			pthread_cond_signal(&bufferAvailable);
			continue;
		} else if (TransmissionState::ERROR == transmissionState) {
			pthread_cond_signal(&bufferAvailable);
			pthread_mutex_unlock(&bufferMutex);
			throw IOException("Error in server thread ");
		} else if (TransmissionState::FINISHED == transmissionState) {
			pthread_cond_signal(&bufferAvailable);
			pthread_mutex_unlock(&bufferMutex);
		}
	}
}

void ClientConnection::startCaching() {
	int pageNumber = 0;
	auto lastPageNumber = 0;
	bool entryFinished = false;
	while(true) {
		CachePage * cachePage = proxy.cacheStorage.getPage(url, pageNumber, lastPageNumber, entryFinished);
		if(pageNumber == lastPageNumber && entryFinished) {
			break;
		}
		writePageToClient(cachePage);
		++pageNumber;
		std::cout << "Send data from cache to client" << std::endl;
	}
}

void ClientConnection::writePageToClient(CachePage *page) {
	auto writeBytes = write(clientSocket, page->data, (size_t) page->size);
	if(-1 == writeBytes) {
		throw IOException("Error writing to client from cache");
	}
}

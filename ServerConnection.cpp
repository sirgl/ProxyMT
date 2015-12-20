#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include "ServerConnection.h"
#include "IOException.h"
#include "tools.h"

void *serverConnectionThreadHelper(ServerConnectionThreadArguments *arguments) {
	ServerConnection connection(arguments->proxy, arguments->connection);
	try {
		connection.start();
	} catch (ProxyException &exception) {
		std::cerr << exception.what() << std::endl;
	}
	delete(arguments);

	return nullptr;
}

ServerConnection::ServerConnection(Proxy &proxy, ClientConnection &connection) :
		proxy(proxy),
		clientConnection(connection) {
}

ServerConnection::~ServerConnection() {
	close(serverSocket);
	clientConnection.serverThreadState = ServerThreadState::NO_THREAD;
	std::cout << "server connection " << serverSocket << " closed" << std::endl;
}

void ServerConnection::start() {
	clientConnection.serverThreadState = ServerThreadState::WORK;
	connectToHost();
	sendRequestToServer();
	receiveResponseFromServer();

	if(isOkResponse()){
		startCachingTransmission();
	} else {
		startNonCachingTransmission();
	}
}

void ServerConnection::connectToHost() {
	struct addrinfo *hostInfo;
	struct addrinfo hint;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;

	int status;
	if ((status = getaddrinfo(clientConnection.host.c_str(), "80", &hint, &hostInfo)) != 0) {
		clientConnection.serverThreadState = ServerThreadState::ERROR;
		notifyClientStateChanged(ServerThreadState::ERROR, TransmissionState::ERROR);
		throw IOException("Error resolving address: " + string(gai_strerror(status)));
	}

	struct sockaddr_in *hostAddress = (sockaddr_in *) hostInfo->ai_addr;
	std::cout << "Connecting to " << clientConnection.host << " (" << getIp(hostAddress) << ")" << std::endl;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == serverSocket) {
		clientConnection.serverThreadState = ServerThreadState::ERROR;
		notifyClientStateChanged(ServerThreadState::ERROR, TransmissionState::ERROR);
		throw IOException("Error creating clientSocket: " + getError(errno));
	}

	if (-1 == connect(serverSocket, (struct sockaddr *) hostAddress, sizeof(*hostAddress))) {
		clientConnection.serverThreadState = ServerThreadState::ERROR;
		notifyClientStateChanged(ServerThreadState::ERROR, TransmissionState::ERROR);
		throw IOException("Error connecting to host: " + getError(errno));
	}
	std::cout << "Server connection " << serverSocket << " created" << std::endl;
}

void ServerConnection::notifyClientStateChanged(ServerThreadState state, TransmissionState transmissionState) {
	pthread_mutex_lock(&clientConnection.serverAnswerMutex);
	clientConnection.serverThreadState = state;
	clientConnection.transmissionState = transmissionState;
	pthread_cond_signal(&clientConnection.serverThreadAnswered);
	pthread_mutex_unlock(&clientConnection.serverAnswerMutex);
}

void ServerConnection::sendRequestToServer() {
	clientConnection.bufferSize = (int) write(serverSocket, clientConnection.buffer, (size_t) clientConnection.bufferSize);
	if(-1 == clientConnection.bufferSize){
		clientConnection.serverThreadState = ServerThreadState::ERROR;
		clientConnection.transmissionState = TransmissionState::ERROR;
		notifyClientStateChanged(ServerThreadState::ERROR, TransmissionState::ERROR);
		throw IOException("Error sending request to server: " + getError(errno));
	}
	clientConnection.bufferSize = 0;
}

void ServerConnection::receiveResponseFromServer() {
	bufferSize = (int) read(serverSocket, buffer, BUFFER_CAPACITY);
	if(-1 == bufferSize) {
		clientConnection.serverThreadState = ServerThreadState::ERROR;
		clientConnection.transmissionState = TransmissionState::ERROR;
		notifyClientStateChanged(ServerThreadState::ERROR, TransmissionState::ERROR);
		throw IOException("Error receiving request to server: " + getError(errno));
	}
}

bool ServerConnection::isOkResponse() {
	return nullptr != strstr(buffer, "200");
}

void ServerConnection::startNonCachingTransmission() {
	copyDataToClientBuffer();
	clientConnection.transmissionType = TransmissionType::NON_CACHING;
	notifyClientStateChanged(ServerThreadState::WORK, TransmissionState::LOADING);
	while (TransmissionState::FINISHED != clientConnection.transmissionState) {
		receiveDataFromServer();
		copyDataToClientBuffer();
	}
}

void ServerConnection::copyDataToClientBuffer() {
	pthread_mutex_lock(&clientConnection.bufferMutex);
	while(clientConnection.bufferSize > 0) {
		pthread_cond_wait(&clientConnection.bufferAvailable, &clientConnection.bufferMutex);
	}
	memcpy(clientConnection.buffer, buffer, (size_t) bufferSize);
	clientConnection.bufferSize = bufferSize;
	bufferSize = 0;
	pthread_cond_signal(&clientConnection.bufferAvailable);
	pthread_mutex_unlock(&clientConnection.bufferMutex);
	std::cout << "Data copied from server thread buffer to client" << std::endl;
}

void ServerConnection::receiveDataFromServer() {
//	assert(bufferSize != 0);
	bufferSize = (int) read(serverSocket, buffer, BUFFER_CAPACITY);
	if(-1 == bufferSize) {
		notifyClientStateChanged(ServerThreadState::ERROR, TransmissionState::ERROR);
		throw IOException("Error reading from server: " + getError(errno));
	}
	if(0 == bufferSize) {
		clientConnection.transmissionState = TransmissionState::FINISHED;
		notifyClientStateChanged(ServerThreadState::WORK, TransmissionState::FINISHED);
	}
	std::cout << "Received data from server" << std::endl;
}

void ServerConnection::startCachingTransmission() {
	proxy.cacheStorage.tryCreateEntry(clientConnection.url);
	copyDataToCache();
	clientConnection.transmissionType = TransmissionType::CACHING;
	notifyClientStateChanged(ServerThreadState::WORK, TransmissionState::LOADING);
	while (TransmissionState::FINISHED != clientConnection.transmissionState) {
		receiveDataFromServer();
		copyDataToCache();
	}
	proxy.cacheStorage.finishEntry(clientConnection.url);
}

void ServerConnection::copyDataToCache() {
	proxy.cacheStorage.addPage(clientConnection.url, new CachePage(buffer, bufferSize));
}

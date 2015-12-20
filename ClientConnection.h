#ifndef PROXYMT_CLIENTCONNECTION_H
#define PROXYMT_CLIENTCONNECTION_H

#include <pthread.h>
#include <string>
#include "Proxy.h"

const int BUFFER_CAPACITY = 16384;

using std::string;

static const char *METHOD_NOT_SUPPORTED = "HTTP/1.0 405";
static const char *HTTP_VERSION_NOT_SUPPORTED = "HTTP/1.0 505";
static const char *BAD_REQUEST = "HTTP/1.0 400";


enum class TransmissionType {
	NO_TRANSMISSION,
	CACHING,
	NON_CACHING
};

enum class TransmissionState {
	LOADING,
	FINISHED,
	ERROR
};

enum class ServerThreadState {
	NO_THREAD,
	WORK,
	ERROR
};

class ClientConnection {
private:
	int clientSocket;
	Proxy& proxy;

	void receiveRequest();


public:
	TransmissionState transmissionState;
	TransmissionType transmissionType;
	ServerThreadState serverThreadState;

	pthread_mutex_t serverAnswerMutex;
	pthread_cond_t serverThreadAnswered;

	pthread_mutex_t bufferMutex;
	pthread_cond_t bufferAvailable;

	char buffer[BUFFER_CAPACITY];
	int bufferSize;

	string url;
	string host;

	ClientConnection(int socket, Proxy &proxy);

	~ClientConnection();

	void start();

	void parseRequest();

	void createServerConnectionThread();

	void waitForServerConnection();

	void startNonCaching();

	void sendDataToClient();

	void startCaching();

	void writePageToClient(CachePage *page);
};

struct ClientConnectionThreadArguments {
	Proxy &proxy;
	int socket;

	ClientConnectionThreadArguments(Proxy &proxy, int socket) :
			proxy(proxy),
			socket(socket) {
	}
};


void *clientConnectionHepler(ClientConnectionThreadArguments *arguments);


#endif //PROXYMT_CLIENTCONNECTION_H

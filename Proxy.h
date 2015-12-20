#ifndef PROXYMT_PROXY_H
#define PROXYMT_PROXY_H


#include <unordered_map>
#include <vector>
#include "cache/CacheStorage.h"

using std::string;
using std::vector;


class Proxy {
	int proxySocket;

	void createClientConnectionThread(int acceptedSocket);
public:
	CacheStorage cacheStorage;

	virtual ~Proxy();

	Proxy(char *listenPort);

	void start();
};


#endif //PROXYMT_PROXY_H

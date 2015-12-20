#pragma once

#include <unordered_map>
#include "CacheEntry.h"

using std::unordered_map;
using std::string;

class CacheStorage {
private:
	unordered_map<string, CacheEntry*> cacheMap;
	pthread_mutex_t mapMutex;

	CacheEntry *getEntry(const string &url);
public:
	CacheStorage();

	bool tryCreateEntry(string url);

	CachePage * getPage(string url, int pageNumber, int &lastPageNumber, bool &entryFinished);

	void addPage(string url, CachePage* page);

	void finishEntry(string url);

	bool contains(string url);
};

#include <iostream>
#include <assert.h>
#include "CacheStorage.h"

CacheStorage::CacheStorage() {
	pthread_mutex_init(&mapMutex, nullptr);
}

bool CacheStorage::tryCreateEntry(string url) {
	pthread_mutex_lock(&mapMutex);
	bool created = false;
	if (cacheMap.find(url) == cacheMap.end()) {
		created = true;
	}
	cacheMap[url] = new CacheEntry();
	pthread_mutex_unlock(&mapMutex);
	return created;
}

CachePage *CacheStorage::getPage(string url, int pageNumber, int &lastPageNumber, bool &entryFinished) {
	pthread_mutex_lock(&mapMutex);
	CacheEntry *entry = cacheMap[url];
	pthread_mutex_unlock(&mapMutex);

	CachePage *pageToReturn;

	pthread_mutex_lock(&entry->mutex);
	while (true) {
		if (pageNumber >= entry->cachePages.size()) {
			if (entry->finished) {
				pageToReturn = nullptr;
				break;
			} else {
				pthread_cond_wait(&entry->cacheEvent, &entry->mutex);
				continue;
			}
		} else {
			pageToReturn = entry->cachePages[pageNumber];
			break;
		}
	}
	lastPageNumber = (int) (entry->cachePages.size() - 1);
	entryFinished = entry->finished;
	pthread_mutex_unlock(&entry->mutex);
	return pageToReturn;
}

void CacheStorage::addPage(string url, CachePage *page) {
	CacheEntry *entry = getEntry(url);

	pthread_mutex_lock(&entry->mutex);
//	assert(!entry->finished);
	entry->cachePages.push_back(page);
	pthread_cond_broadcast(&entry->cacheEvent);
	pthread_mutex_unlock(&entry->mutex);
}

void CacheStorage::finishEntry(string url) {
	CacheEntry *entry = getEntry(url);
	assert(entry != nullptr);
	pthread_mutex_lock(&entry->mutex);
	entry->finished = true;
	pthread_cond_signal(&entry->cacheEvent);
	pthread_mutex_unlock(&entry->mutex);
}

CacheEntry *CacheStorage::getEntry(const string &url) {
	pthread_mutex_lock(&mapMutex);
	CacheEntry *entry = cacheMap[url];
	pthread_mutex_unlock(&mapMutex);
	return entry;
}

bool CacheStorage::contains(string url) {
	pthread_mutex_lock(&mapMutex);
	bool result = cacheMap.find(url) != cacheMap.end();
	pthread_mutex_unlock(&mapMutex);
	return result;
}

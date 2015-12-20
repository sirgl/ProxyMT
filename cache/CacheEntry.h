#pragma once

#include <vector>
#include <utility>
#include <string>
#include <string.h>

struct CachePage {
	char* data;
	int size;

	CachePage(char *data, int size) : size(size) {
		this->data = new char[size];
		memcpy(this->data, data, (size_t) size);
	}
};

struct CacheEntry {

	CacheEntry() {
		pthread_mutex_init(&mutex, nullptr);
		pthread_cond_init(&cacheEvent, nullptr);
		finished = false;
	}
	bool finished;
	std::vector<CachePage*> cachePages;
	pthread_mutex_t mutex;

	pthread_cond_t cacheEvent;
};

#include <iostream>
#include <sstream>
#include "cache/CacheStorage.h"

using namespace std;

CacheStorage storage;

void *producer(void *) {
	int i = 0;
	while (true) {
		std::stringstream ss;
		ss << i;
		char *str = (char *) ss.str().c_str();
		storage.addPage("hello", new CachePage(str, (int) strlen(str)));
		if (i == 100000) {
			break;
		}
		++i;
	}
	storage.finishEntry("hello");
}


void *consumer(void *) {
	int i = 0;
	while (true) {
		int lastPage = 0;
		bool entryFinished = false;
		auto pPage = storage.getPage("hello", i, lastPage, entryFinished);
		std::cout << string(pPage->data, (unsigned long) pPage->size) << std::endl;
		if (lastPage == i && entryFinished) {
			return nullptr;
		}
		++i;
	}
}

int main() {

	pthread_t thread1;
	pthread_t thread2;
	storage.tryCreateEntry("hello");

	pthread_create(&thread1, nullptr, producer, nullptr);
	pthread_create(&thread2, nullptr, consumer, nullptr);
	pthread_exit(0);
}
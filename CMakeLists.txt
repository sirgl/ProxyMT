cmake_minimum_required(VERSION 3.3)
project(ProxyMT)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
find_package(Threads)

set(SOURCE_FILES main.cpp cache/CacheEntry.h cache/CacheStorage.cpp cache/CacheStorage.h
        IOException.h ProxyException.h SocketException.h tools.cpp tools.h parser/RequestParser.cpp
        parser/Request.h parser/RequestParser.h ClientConnection.cpp ClientConnection.h Proxy.cpp Proxy.h ServerConnection.cpp ServerConnection.h)

add_executable(ProxyMT ${SOURCE_FILES})
target_link_libraries(ProxyMT ${CMAKE_THREAD_LIBS_INIT})
#pragma once
#include <string>
#include <vector>
#include <netinet/in.h>


std::string getError(int error);
std::vector<std::string> split(std::string str, char delimiter);
std::string getIp(sockaddr_in * socketAddress);

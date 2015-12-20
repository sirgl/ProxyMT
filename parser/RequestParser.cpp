#include "RequestParser.h"
#include "../tools.h"
#include <algorithm>
#include <sstream>


RequestParser::RequestParser(const string &requestData) :
		requestData(requestData) {
	position = 0;
}

Request* RequestParser::parse() {
	Request *request = new Request();
	std::string line;
	if(!tryGetNextLine(line)){
		throw ParserException("Bad request");
	}
	auto requestLineTokens = split(line, ' ');
	if(requestLineTokens.size() < 3) {
		throw ParserException("Bad request line");
	}
	request->method = requestLineTokens[0];
	request->url = requestLineTokens[1];
	request->protocol = requestLineTokens[2];
	line.clear();
	while (tryGetNextLine(line)) {
		if(line.empty()) {
			break;
		}
		auto headerTokens = split(line, ':');
		request->headers[headerTokens[0]] = headerTokens[1];
		line.clear();
	}
	request->data = getLeftData();
	return request;
}


bool RequestParser::tryGetNextLine(std::string &line) {
	auto lineIterator = requestData.cbegin() + position;
	auto iterator = std::find(lineIterator, requestData.cend(), '\n');
	if (iterator == requestData.cend()) {
		return false;
	}
	int offset = 0;
	if (lineIterator != iterator && *(iterator - 1) == '\r') {
		offset = -1;
	}
	std::copy(lineIterator, iterator + offset, std::back_inserter(line));
	position = (unsigned int) (iterator - requestData.begin() + 1);
	return true;
}


string RequestParser::getLeftData() {
	string data;
	auto iterator = requestData.begin() + position;
	std::copy(iterator, requestData.end(), std::back_inserter(data));
	return data;
}

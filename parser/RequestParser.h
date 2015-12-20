#ifndef REQUEST_PARSER
#define REQUEST_PARSER


#include <vector>
#include <unordered_map>
#include <string>
#include "Request.h"
#include "../ProxyException.h"

using std::string;

class RequestParser{
	string requestData;
	unsigned int position;

	string getLeftData();
	bool tryGetNextLine(string &line);
public:
	RequestParser(const string &requestData);
	Request* parse();
};

class ParserException : public ProxyException {
public:
	ParserException(const std::string &reason) : ProxyException(reason) { }
};

#endif

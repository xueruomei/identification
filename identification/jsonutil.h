#pragma once
#include <string>
#include <sstream>
using namespace std;
static string yy(const char * key, const char * value)
{
	string ret = "\"";
	ret += key;
	ret += "\"";
	ret += ":";
	ret += "\"";
	ret += value;
	ret += "\"";
	return ret;
}
static string yy(const char * key, int num)
{
	stringstream s;
	s << "\"" << key << "\":";
	s << num;
	return s.str();
}
static string yy(const char * key, double num)
{
	stringstream s;
	s << "\"" << key << "\":";
	s << num;
	return s.str();
}
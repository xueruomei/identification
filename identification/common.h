#pragma once


#ifdef _WIN32
#include <Windows.h>
#endif
#include <string>
#include <vector>

using namespace std;
template<typename T>
static std::string itostr(const T& t)
{
	std::ostringstream oss;
	oss << t;
	return oss.str();
}

template<typename T>
static T strto(const std::string& s)
{
	std::istringstream iss(s);
	T x;
	if (!(iss >> x))
		return 0;
	return x;
}


static void Split(const string& strSource, char& spec, std::vector<string> &strlist)
{
#define END string::npos 
	std::string Temp = strSource;
	std::string::size_type  n = Temp.size();
	if (Temp[--n] != spec)
		Temp.push_back(spec);
	if (Temp[0] == spec)
		Temp = Temp.substr(1);
	strlist.clear();
	string::size_type post = 0;
	string::size_type posf = 0;
	while ((post = Temp.find_first_of(spec, post)) != END)
	{
		string strt = "";
		while (posf != post)
		{
			strt.push_back(Temp[posf]);
			posf++;
		}
		//string strt = Temp.substr(posf,post);
		strlist.push_back(strt);
		posf++;
		if (++post == END)
			break;
	}
}

static string UnicodeToUTF8(const wstring& str)
{
	char*     pElementText;
	int    iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte(CP_UTF8,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
	::WideCharToMultiByte(CP_UTF8,
		0,
		str.c_str(),
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL);
	string strText;
	strText = pElementText;
	delete[] pElementText;
	return strText;
}
static string Utf82Gb2312(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	string ret = str;
	delete[]str;
	return ret;
}
static string GB23122Utf8(const char* gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	string ret = str;
	delete[]str;
	return ret;
}


static void OutMessage(const char * fmt, ...)
{
#ifdef _WIN32
	va_list args;
	va_start(args, fmt);

	char buf[4096];
	_vsnprintf_s(buf, 512, fmt, args);
	va_end(args);
	string str = "BROAD:";
	str += buf;
	OutputDebugStringA(str.c_str());
#endif
}
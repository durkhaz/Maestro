#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

std::wstring WidenString(const std::string &s)
{
	std::wstring wsTmp(s.begin(), s.end());
	return wsTmp;
}

std::string NarrowString(const std::wstring &s)
{
	std::string wsTmp(s.begin(), s.end());
	return wsTmp;
}
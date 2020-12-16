#include <Windows.h>
#include "Convert.h"

namespace webinar
{
	Convert::Convert()
	{
	}


	Convert::~Convert()
	{
	}

	std::string Convert::WstrToStr(const std::wstring &wstr)
	{
		std::string str(wstr.length() + 1, '\0');
		int iConvertedSize = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wstr.c_str(), -1, &str[0], str.size(), 0, 0);
		str.resize(iConvertedSize);
		return str;
	}

	int Convert::WstrToStr(std::string &str, const std::wstring &wstr, const int & ConvertCount)
	{
		str.resize(wstr.length() + 1, '\0');
		int iConvertedSize = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wstr.c_str(), ConvertCount, &str[0], str.size(), 0, 0);
		str.resize(iConvertedSize);
		return iConvertedSize;
	}

	std::wstring Convert::StrToWStr(const std::string &str)
	{
		std::wstring wstr(str.length() + 1, '\0');
		int iConvertedSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), str.length(), &wstr[0], wstr.size());
		wstr.resize(iConvertedSize);
		return wstr;
	}

	int Convert::StrToWstr(std::wstring &wstr, const std::string &str)
	{
		wstr.resize(str.length() + 1, '\0');
		int iConvertedSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), str.length(), &wstr[0], wstr.size());
		wstr.resize(iConvertedSize);
		return iConvertedSize;
	}
}
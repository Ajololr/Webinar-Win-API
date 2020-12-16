#pragma once
#include <string>

namespace webinar
{
	class Convert
	{
	public:
		Convert();
		virtual ~Convert();

		static std::string WstrToStr(const std::wstring &);
		static int WstrToStr(std::string&, const std::wstring &, const int & ConvertCount = -1);

		static std::wstring StrToWStr(const std::string &);
		static int StrToWstr(std::wstring&, const std::string &);
	};
}
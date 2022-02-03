#include <iostream>
#include <locale>
#include <string>
#include <fstream>
#include <vector>
#include <windows.h>

using namespace std;


std::string convert(wchar_t const *s, size_t len)
{
	int n = WideCharToMultiByte(CP_UTF8, 0, s, len, 0, 0, 0, 0);
	if (n == 0)
	{
		throw std::runtime_error("bad conv");
	}
	std::vector<char> buf(n);
	WideCharToMultiByte(CP_UTF8, 0, s, len, &buf[0], n, 0, 0);
	return std::string(&buf[0], n);
}
std::wstring convert(char const* s, size_t len)
{
	int n = MultiByteToWideChar(CP_UTF8, 0, s, len, 0, 0);
	if (n <= 0)
	{
		throw std::runtime_error("bad conv");
	}
	std::vector<wchar_t> buf(n);
	n = MultiByteToWideChar(CP_UTF8, 0, s, len, &buf.front(), n);
	return std::wstring(&buf[0], n);
}

std::string WStringToString(const std::wstring &wstr)
{
      std::string str(wstr.length(), ' ');
      std::copy(wstr.begin(), wstr.end(), str.begin());
      return str;
}

void readUtf8File(const char* file, std::vector<char>* content )
{
	//utf.txt is encoded as utf-8 with bom
	std::ifstream ifs(file, std::ifstream::binary);
	//char bom[3]; 
	//ifs.read(bom, sizeof(bom));

	char ch;
	while (ifs.read(&ch, 1)) {
		content->push_back(ch);
	}

	printf("%s", content->data());

	//to show it in console that has ansi/OEM GBK, convert it in wide char string 
	//std::locale::global(std::locale(".936"));
	//std::wstring wstr = convert(&content[0], content.size());
	//std::wcout << wstr << std::endl;
	

	//std::string result;
	//result.assign(wstr.begin(), wstr.end());


	//save it back in utf-8
	//std::ofstream ofs("out.txt", std::ofstream::binary);
	//ofs.write(bom, sizeof(bom));
	//std::string str = convert(wstr.c_str(), wstr.length());
	//ofs.write(str.c_str(), str.length());

	return;
}
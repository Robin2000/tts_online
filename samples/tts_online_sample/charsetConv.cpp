#include <string>
#include <codecvt>
#include <locale>
#include <vector>
#include "charsetConv.h"

using  namespace std;

char *utf8ToString(char *utf8str)
{
	// UTF-8 to wstring
	wstring_convert<codecvt_utf8<wchar_t>> wconv;
	wstring wstr = wconv.from_bytes(utf8str);
	// wstring to string
	vector<char> buf(wstr.size());
	const locale& loc = locale(".1252");
	
	use_facet<ctype<wchar_t>>(loc).narrow(wstr.data(), wstr.data() + wstr.size(), '?', buf.data());

	return buf.data();
}
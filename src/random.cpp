
#include <string>
#include <cstdlib>
#include <time.h>

std::string gen_random(const int len)
{
	static const char alphanum[] =
	"âêîôûŷéèàäëïöüÿ"
	"0123456789\n\t\r"
	"æ«€¶ŧ←↓→øþ@ßðđŋħĸłµł»¢“”n─·¤¬"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"²&~\"#'[{()}]-|`_\\ç^@°+=$£%*µ?,.;/:§!<>"
	"abcdefghijklmnopqrstuvwxyz"
	" Æ¢®Ŧ¥↑ıØÞΩÐªŊĦŁ©×÷¡⅛⅜⅝⅞™±¿";
	std::string tmp_s;
	tmp_s.reserve(len);


	std::srand(time(NULL));

	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[std::rand() % (sizeof(alphanum) - 1)];
	}

	return tmp_s;
}

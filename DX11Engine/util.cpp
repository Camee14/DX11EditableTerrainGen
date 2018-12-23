#include "utils.h"

std::vector<std::string> splitString(std::string s, std::regex r) {
	std::vector<std::string> tokens;
	std::sregex_token_iterator begin(s.begin(), s.end(), r), end;
	std::copy(begin, end, std::back_inserter(tokens));

	return tokens;
}
// Author: https://github.com/AMXerSurf
#include "StringUtils.h"

#include <cctype>
#include <uni-algo/uni_algo.h>

using namespace AEX;

//############################ StringUtils ############################

void StringUtils::split(const std::string& string, const std::string& splitSequence, ArrayList<std::string>& splitParts, uint32_t splitPartsCount, bool ignoreEmptyLast) {
	if(splitPartsCount < 1) {
		return;
	}
	if(splitPartsCount < 2) {
		splitParts.add(string);
		return;
	}
	std::string currentString = string;
	una::search currentOccurrence = una::casesens::search_utf8(currentString, splitSequence);
	uint32_t index = 0;
	while(currentOccurrence && index < splitPartsCount - 1) {
		splitParts.add(currentString.substr(0, currentOccurrence.pos()));
		currentString = currentString.substr(currentOccurrence.pos() + splitSequence.size());
		currentOccurrence = una::casesens::search_utf8(currentString, splitSequence);
		index++;
	}
	if(!ignoreEmptyLast || currentString.length() > 0) {
		splitParts.add(currentString);
	}
}

uint32_t StringUtils::split(const std::string& string, const std::string& splitSequence, std::string* splitParts, uint32_t splitPartsCount) {
	if(splitPartsCount < 1) {
		return 0;
	}
	if(splitPartsCount < 2) {
		splitParts[0] = string;
		return 1;
	}
	std::string currentString = string;
	una::search currentOccurrence = una::casesens::search_utf8(currentString, splitSequence);
	uint32_t index = 0;
	while(currentOccurrence && index < splitPartsCount - 1) {
		splitParts[index++] = currentString.substr(0, currentOccurrence.pos());
		currentString = currentString.substr(currentOccurrence.pos() + splitSequence.size());
		currentOccurrence = una::casesens::search_utf8(currentString, splitSequence);
	}
	splitParts[index++] = currentString;
	return index;
}

ArrayList<std::string> StringUtils::scriptSplit(const std::string& string, const std::string& splitSequence, uint32_t splitPartsCount, bool ignoreEmptyLast) {
	ArrayList<std::string> splitParts;
	split(string, splitSequence, splitParts, splitPartsCount, ignoreEmptyLast);
	return splitParts;
}

void StringUtils::replace(std::string& string, const std::string& from, const std::string& to) {
	if(!from.empty()) {
		size_t startPosition = 0;
		una::search searchResult;
		while((searchResult = una::casesens::search_utf8(std::string_view(&string.data()[startPosition], string.length() - startPosition), from))) {
			startPosition += searchResult.pos();
			string.replace(startPosition, from.length(), to);
			startPosition += to.length();
		}
	}
}

std::string StringUtils::replaced(const std::string& string, const std::string& from, const std::string& to) {
	std::string stringCopy = string;
	replace(stringCopy, from, to);
	return stringCopy;
}

bool StringUtils::equals(const std::string& string, const std::string& compareString) {
	if(string.size() != compareString.size()) {
		return false;
	}
	return una::casesens::compare_utf8(string, compareString) == 0;
}

bool StringUtils::equalsIgnoreCase(const std::string& string, const std::string& compareString) {
	if(string.size() != compareString.size()) {
		return false;
	}
	return una::caseless::compare_utf8(string, compareString) == 0;
}

bool StringUtils::startsWith(const std::string& string, const std::string& prefix) {
	if(string.length() < prefix.length()) {
		return false;
	}
	una::search result = una::casesens::search_utf8(string, prefix);
	if(!result) {
		return false;
	}
	return result.pos() == 0;
}

bool StringUtils::contains(const std::string& string, const std::string& pattern) {
	if(string.length() < pattern.length()) {
		return false;
	}
	if(!una::casesens::search_utf8(string, pattern)) {
		return false;
	}
	return true;
}

bool StringUtils::endsWith(const std::string& string, const std::string& suffix) {
	std::u32string utf32String = convertUTF8ToUTF32(string);
	std::u32string utf32Suffix = convertUTF8ToUTF32(suffix);
	if(utf32String.length() < utf32Suffix.length()) {
		return false;
	}
	return !utf32String.compare(utf32String.length() - utf32Suffix.length(), utf32Suffix.length(), utf32Suffix);
}

void StringUtils::trim(std::string& string) {
	std::u32string utf32String = convertUTF8ToUTF32(string);
	std::u32string::const_iterator leftIterator = utf32String.begin();
	while(leftIterator != utf32String.end() && una::codepoint::is_whitespace(*leftIterator)) {
		leftIterator++;
	}
	std::u32string::const_reverse_iterator rightIterator = utf32String.rbegin();
	while(rightIterator.base() != leftIterator && una::codepoint::is_whitespace(*rightIterator)) {
		rightIterator++;
	}
	string = convertUTF32ToUTF8(std::u32string(leftIterator, rightIterator.base()));
}

std::string StringUtils::trimd(const std::string& string) {
	std::string stringCopy = string;
	trim(stringCopy);
	return stringCopy;
}

std::string StringUtils::demangleClassName(const std::string& mangledClassName) {
#ifndef AEX_PLATFORM_LINUX
	ArrayList<std::string> splitParts;
	split(replaced(replaced(mangledClassName, "struct ", ""), "class ", ""), "::", splitParts);
	return splitParts[splitParts.size() - 1];
#else
	int status = -4;
	std::unique_ptr<char, void(*)(void*)> result(abi::__cxa_demangle(mangledClassName.c_str(), NULL, NULL, &status), std::free);
	ArrayList<std::string> splitParts;
	split(replaced(StringUtils::replaced((status == 0) ? std::string(result.get()) : mangledClassName, "struct ", ""), "class ", ""), "::", splitParts);
	return splitParts[splitParts.size() - 1];
#endif
}

void StringUtils::toLowerCase(std::string& string) {
	string = una::cases::to_lowercase_utf8(string);
}

std::string StringUtils::toLowerCased(const std::string& string) {
	return una::cases::to_lowercase_utf8(string);
}

void StringUtils::toUpperCase(std::string& string) {
	string = una::cases::to_uppercase_utf8(string);
}

std::string StringUtils::toUpperCased(const std::string& string) {
	return una::cases::to_uppercase_utf8(string);
}

void StringUtils::cut(std::string& string, uint32_t characterCount, uint32_t firstCharacter) {
	//Convert to UTF32 -> substring -> convert to UTF8
	std::u32string utf32String = convertUTF8ToUTF32(string);
	std::u32string substring = utf32String.substr(firstCharacter, characterCount);
	string = convertUTF32ToUTF8(substring);
}

std::string StringUtils::substring(const std::string& string, uint32_t characterCount, uint32_t firstCharacter) {
	std::string stringCopy = string;
	cut(stringCopy, characterCount, firstCharacter);
	return stringCopy;
}

std::string StringUtils::convertUTF32CodepointToUTF8(uint32_t codepoint) {
	//Kinda ugly, I know
	char32_t utf32Codepoints[2] = { static_cast<char32_t>(codepoint), 0 };
	std::u32string string = std::u32string(utf32Codepoints);
	return una::utf32to8<char32_t, char>(string);
}

std::string StringUtils::convertUTF32ToUTF8(const std::u32string& string) {
	return una::utf32to8<char32_t, char>(string);
}

std::string StringUtils::convertUTF16ToUTF8(const std::u16string& string) {
	return una::utf16to8<char16_t, char>(string);
}

std::u32string StringUtils::convertUTF8ToUTF32(const std::string& string) {
	return una::utf8to32<char, char32_t>(string);
}

std::u16string StringUtils::convertUTF8ToUTF16(const std::string& string) {
	return una::utf8to16<char, char16_t>(string);
}

uint32_t StringUtils::getStringLength(const std::string& string) {
	return static_cast<uint32_t>(convertUTF8ToUTF32(string).length());
}
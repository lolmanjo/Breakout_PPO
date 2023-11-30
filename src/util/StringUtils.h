// Author: https://github.com/AMXerSurf
#pragma once

#include <string>
#include "Collections.h"

namespace AEX {

	/**
	* Utility class for string manipulation
	* Contains various string related functions including UTF conversion
	*/
	class StringUtils {
		public:
			/**
			* UTF-8 aware split function
			* Splits the input string every time it finds the given split sequence up
			* @param string The string to split
			* @param splitSequence The sequence to split at
			* @param splitParts The list in which the split parts should be inserted
			* @param splitPartsCount The maximum amount of splits to make (optional, default = UINT32_MAX)
			* @param ignoreEmptyLast Defines if the last split part should be discarded if its just an empty string (optional, default = false)
			*/
			static void split(const std::string& string, const std::string& splitSequence, ArrayList<std::string>& splitParts, uint32_t splitPartsCount = UINT32_MAX, bool ignoreEmptyLast = false);
			
			/**
			* UTF-8 aware split function
			* Splits the input string every time it finds the given split sequence up
			* This version requires a fixed maximum and a string array that is managed by the user
			* @param string The string to split
			* @param splitSequence The sequence to split at
			* @param splitParts The string array that will be filled with the split parts
			* @param splitPartsCount The maximum amount of splits to make
			* @return The number of splits that were performed
			*/
			static uint32_t split(const std::string& string, const std::string& splitSequence, std::string* splitParts, uint32_t splitPartsCount);
			
			/**
			* UTF-8 aware split function
			* Splits the input string every time it finds the given split sequence up
			* This function is primarly targeted for Lua use. Use the other two functions in C++, they are a bit more efficient.
			* @param string The string to split
			* @param splitSequence The sequence to split at
			* @param splitPartsCount The maximum amount of splits to make (optional, default = UINT32_MAX)
			* @param ignoreEmptyLast Defines if the last split part should be discarded if its just an empty string (optional, default = false)
			* @return A list with all the split parts
			*/
			static ArrayList<std::string> scriptSplit(const std::string& string, const std::string& splitSequence, uint32_t splitPartsCount = UINT32_MAX, bool ignoreEmptyLast = false);
			
			/**
			* UTF-8 aware in place replace function
			* Replaces all occurrences of the given pattern with the given replacement pattern
			* @param string The string to modify (functions as input AND output)
			* @param from The pattern to replace
			* @param to The replacement pattern
			*/
			static void replace(std::string& string, const std::string& from, const std::string& to);

			/**
			* UTF-8 aware replace function
			* Replaces all occurrences of the given pattern with the given replacement pattern
			* This version returns a seperate new string in which all occurrences where replaced (the input won't be modified)
			* @param string The string to modify (functions as input AND output)
			* @param from The pattern to replace
			* @param to The replacement pattern
			* @return The string in which all occurrences where replaced
			*/
			static std::string replaced(const std::string& string, const std::string& from, const std::string& to);

			/**
			* UTF-8 aware string comparison function
			* Example: StringUtils::equals("TeSt", "test") returns false
			* @param string The string to compare
			* @param compareString The string to compare to
			* @return true if the strings are equal, otherwise false
			*/
			static bool equals(const std::string& string, const std::string& compareString);

			/**
			* UTF-8 aware string comparison function that ignores the casing of the characters
			* Example: StringUtils::equalsIgnoreCase("TeSt", "test") returns true
			* @param string The string to compare
			* @param compareString The string to compare to
			* @return true if the strings are equal (ignoring casing), otherwise false
			*/
			static bool equalsIgnoreCase(const std::string& string, const std::string& compareString);

			/**
			* UTF-8 aware starts with function that checks if the passed string starts with the passed prefix
			* @param string The string to check
			* @param prefix Beginning to check
			* @return true if string starts with prefix, otherwise false
			*/
			static bool startsWith(const std::string& string, const std::string& prefix);

			/**
			* UTF-8 aware contains function that checks if the passed string pattern is contained in the passed string
			* @param string The string to check
			* @param pattern The string to find
			* @return true if string contains pattern, otherwise false
			*/
			static bool contains(const std::string& string, const std::string& pattern);

			/**
			* UTF-8 aware ends with function that checks if the passed string ends with the passed suffix
			* @param string The string to check
			* @param suffix Ending to check
			* @return true if string ends with suffix, otherwise false
			*/
			static bool endsWith(const std::string& string, const std::string& suffix);

			/**
			* UTF-8 aware in place trim function
			* Removes all "space" characters in front of and at the end of the string so that only the actually relevent content in the middle remains
			* @param string The string to modify (functions as input AND output)
			*/
			static void trim(std::string& string);

			/**
			* UTF-8 aware trim function
			* Removes all "space" characters in front of and at the end of the string so that only the actually relevent content in the middle remains
			* This version returns a seperate new string that is trimmed (the input won't be modified)
			* @param string The string to trim
			* @return The trimmed string
			*/
			static std::string trimd(const std::string& string);

			/**
			* "Demangles" the class name from typeid(Class).name()
			*  Example (Windows): 
			*		typeid(StringUtils).name() -> "class AEX::StringUtils"
			*		StringUtils::demangleClassName(typeid(StringUtils).name()) -> "StringUtils"
			*  This also removes other weird things from the class name (like on Linux where typeid(...).name() also contains a hash)
			*  So Tldr: It extracts the raw class/struct name from typeid(...).name()
			*  @param mangledClassName The mangled name (usually from typeid(T).name())
			*/
			static std::string demangleClassName(const std::string& mangledClassName);

			/**
			* UTF-8 aware lower case in place conversion function
			* Converts every character (if possible for that character) to its lower case version
			* @param string The string to modify (functions as input AND output)
			*/
			static void toLowerCase(std::string& string);

			/**
			* UTF-8 aware lower case conversion function
			* Converts every character (if possible for that character) to its lower case version
			* This version returns a seperate new string that is lower cased (the input won't be modified)
			* @param string The string to convert to lower case
			* @return The "lower cased" string
			*/
			static std::string toLowerCased(const std::string& string);

			/**
			* UTF-8 aware upper case in place conversion function
			* Converts every character (if possible for that character) to its upper case version
			* @param string The string to modify (functions as input AND output)
			*/
			static void toUpperCase(std::string& string);

			/**
			* UTF-8 aware upper case conversion function
			* Converts every character (if possible for that character) to its upper case version
			* This version returns a seperate new string that is upper cased (the input won't be modified)
			* @param string The string to convert to upper case
			* @return The "upper cased" string
			*/
			static std::string toUpperCased(const std::string& string);

			/**
			* UTF-8 aware substring function
			* The problem with the normal substr function of std::string is that its not UTF-8 aware
			*  which is a problem for characters that are made up of multiple UTF-8 elements
			* @param string The UTF-8 encoded input string
			* @param characterCount The character count of the substring
			* @param firstCharacter A character offset from which to start the substring from (optional, default = 0)
			*/
			static void cut(std::string& string, uint32_t characterCount, uint32_t firstCharacter = 0);

			/**
			* UTF-8 aware substring function
			* The problem with the normal substr function of std::string is that its not UTF-8 aware 
			*  which is a problem for characters that are made up of multiple UTF-8 elements
			* @param string The UTF-8 encoded input string
			* @param characterCount The character count of the substring
			* @param firstCharacter A character offset from which to start the substring from (optional, default = 0)
			* @return The substring from firstCharacter (inclusive) to (firstCharacter + characterCount) (exclusive)
			*/
			static std::string substring(const std::string& string, uint32_t characterCount, uint32_t firstCharacter = 0);

			/**
			* Creates a UTF-8 encoded string containing the character with the requested codepoint
			* This can be used for example in Lua to append any unicode character to a string
			* @param The codepoint of the character to convert
			* @return The string containing the character
			*/
			static std::string convertUTF32CodepointToUTF8(uint32_t codepoint);

			/**
			* Converts the given string into a UTF-8 encoded string (std::string)
			* Important: The input string has to be encoded with UTF-32!
			* @param string The UTF-32 encoded input string
			* @return The UTF-8 encoded result
			*/
			static std::string convertUTF32ToUTF8(const std::u32string& string);

			/**
			* Converts the given string into a UTF-8 encoded string (std::string)
			* Important: The input string has to be encoded with UTF-16!
			* @param string The UTF-16 encoded input string
			* @return The UTF-8 encoded result
			*/
			static std::string convertUTF16ToUTF8(const std::u16string& string);

			/**
			* Converts the given string into a UTF-32 encoded string (std::u32string)
			* Important: The input string has to be encoded with UTF-8!
			* @param string The UTF-8 encoded input string
			* @return The UTF-32 encoded result
			*/
			static std::u32string convertUTF8ToUTF32(const std::string& string);

			/**
			* Converts the given string into a UTF-16 encoded string (std::u16string)
			* Important: The input string has to be encoded with UTF-8!
			* @param string The UTF-8 encoded input string
			* @return The UTF-16 encoded result
			*/
			static std::u16string convertUTF8ToUTF16(const std::string& string);

			/**
			* Counts the number of characters for a UTF-8 encoded string
			* Important: The input string has to be encoded with UTF-8!
			* @param string The UTF-8 encoded input string
			* @return The number of characters (unicode)
			*/
			static uint32_t getStringLength(const std::string& string);
		protected:
		private:
	};
}
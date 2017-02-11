/**
 * @file tokens.hpp
 * @brief Provides a way to iterate through tokens in a string.
 */

#ifndef TOKENS_HPP_
#define TOKENS_HPP_

#include <string>

/**
 * @class tokens
 * A class to provide the ability to iterate through parts of a string, with a
 * given delimiting character.
 */
class tokens {
private:
	/**
	 * A reference of the string to iterate through.
	 */
	const std::string& str;

	/**
	 * The delimiting character, to split the string into tokens.
	 */
	char delim;

public:
	/**
	 * @class iterator
	 * Provides a method of iterating through the tokens in the string.
	 */
	class iterator {
	public:

		/** The string to iterate through. */
		const std::string& str;

		/** The delimiting character. */
		char delim;

		/** The current index in the string. */
		unsigned int index;

		iterator(unsigned int i, const std::string& s = "", char d = 0)
			: str(s), delim(d), index(i) {}

		inline bool operator!=(const iterator& i) {
			return index != i.index;
		}

		inline iterator& operator++(void) {
			index++;
			return *this;
		}

		std::string operator*(void) {
			std::string token;
			while (index < str.size()) {
				if (str[index] == delim)
					return token;
				else
					token += str[index++];
			}

			return token;
		}
	};

	tokens(const std::string& s, char d)
		: str(s), delim(d) {}

	inline iterator begin(void) const {
		return iterator(0, str, delim);
	}

	inline iterator end(void) const {
		return iterator(str.size() + 1, str, delim);
	}
};


#endif // TOKENS_HPP_

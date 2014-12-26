#ifndef __SYNTAXERROR_H__
#define __SYNTAXERROR_H__

#include <exception>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>

class SyntaxError : public std::exception
{
public:
	static const int MaximumErrorMessageLength = 512;

public:
	SyntaxError(const SourceCode &c, int fallback, const char *fmt, ...) {
		std::ifstream fin(c.getPath());
		int pos = c.getPosition() - fallback;
		int line = 1, column = pos;
		std::string s;
		std::ostringstream msg;

		msg << std::endl;
		while(getline(fin, s)) {
			if(column > s.length())
				column -= s.length() + 1;
			else break;
			line++;
		}

		msg << c.getPath() << ':' << line << ':' << column << ": error: ";
		int start = s.find_first_not_of(" \t"), end = s.find_last_not_of(" \t");
		column -= start;
		s = s.substr(start, end - start + 1);

		char buffer[MaximumErrorMessageLength + 1];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, MaximumErrorMessageLength, fmt, args);
		va_end(args);

		msg << buffer << "\n  " << s << "\n  ";
		for(int i = 0; i < column; i++)
			msg << ' ';
		msg << '^' << std::endl;
		fin.close();
		msgstr = msg.str();
	}

	const char *what() const noexcept {
		return msgstr.c_str();
	}

private:
	std::string msgstr;
};

#endif // __SYNTAXERROR_H__
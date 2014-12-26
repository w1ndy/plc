#ifndef __SOURCECODE_H__
#define __SOURCECODE_H__

#include <cstdio>
#include <stack>
#include <string>

class SourceCode
{
public:
	SourceCode();
	virtual ~SourceCode();

	bool open(const char *path);
	void close();

	char read();
	void undo(char ch);

	inline SourceCode &operator>>(char &ch) {
		ch = read();
		return *this;
	}

	inline SourceCode &operator<<(char ch) {
		undo(ch);
		return *this;
	}

	inline int getPosition() const {
		return position;
	}

	inline const char *getPath() const {
		return filepath.c_str();
	}

private:
	const static unsigned int MaximumInputBufferLength	= 80;
	const static unsigned int MaximumUndoBufferLength	= 80;

	std::string filepath;

	FILE *	fin;
	char	input_buf[MaximumInputBufferLength + 1];
	char	undo_buf[MaximumUndoBufferLength + 1];
	int 	input_buf_len, input_buf_ptr, undo_buf_ptr, position;
};

#endif // __SOURCECODE_H__
#include "SourceCode.h"
using namespace std;

SourceCode::SourceCode() : fin(NULL)
{
}

SourceCode::~SourceCode()
{
	close();
}

bool SourceCode::open(const char *path)
{
	close();
	filepath = path;
	return ((fin = fopen(path, "r")) != NULL);
}

void SourceCode::close()
{
	if(fin) fclose(fin);
	fin = NULL;
	input_buf_len = input_buf_ptr = undo_buf_ptr = position = 0;
}

char SourceCode::read()
{
	if(fin == NULL)
		return 0;

	if(undo_buf_ptr != 0) {
		position++;
		return undo_buf[--undo_buf_ptr];
	}
	
	if(input_buf_ptr == input_buf_len) {
		input_buf_len = fread(input_buf, 1, 80, fin);
		input_buf_ptr = 0;
		if(input_buf_len <= 0) {
			return 0;
		}
	}

	position++;
	return input_buf[input_buf_ptr++];
}

void SourceCode::undo(char ch)
{
	undo_buf[undo_buf_ptr++] = ch;
	position--;
}

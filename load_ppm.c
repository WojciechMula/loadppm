#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

char read_int(FILE* f, int* v) {
	int value = 0;	// readed value
	int c;		// buffer
	char valid = 0;	// flag
	
	while (1) {
		c = fgetc(f);
		if (c >= '0' && c <= '9') {
			value = 10*value + (c - '0');
			valid = 1;
		}
		else {
			ungetc(c, f);
			break;
		}
	}
	
	if (valid) // alter variable only if succed
		*v = value;
	return valid;
}

char skip_whitespaces(FILE* f) {
	char skipped = 0;
	int c;
	while (1) {
		c = getc(f);
		if (c == '\n' || c == '\t' || c == '\r' || c == ' ')
			skipped = 1;
		else {
			ungetc(c, f);
			break;
		}
	}
	return skipped;
}

char skip_comment(FILE* f) {
	int c;
	c = fgetc(f);
	if (c != '#') {
		ungetc(c, f);
		return 0;
	}
	while (c != '\n')
		c = fgetc(f);
	
	if (c == EOF)
		return 0;
	else
		return 1;
}

char* PPM_errmsg[] = {
	/* 0 */ "no error",
	/* 1 */ "wrong signature",
	/* 2 */ "image width absent",
	/* 3 */ "image height absent",
	/* 4 */ "image maxval absent",
	/* 5 */ "no single whitespace after maxval",
	/* 6 */ "maxval larger then 65535",
	/* 7 */ "no free memory for image pixels",
	/* 8 */ "too few image pixels"
};

int load_ppm(FILE* f, int* width, int* height, int* maxval, uint8_t** data) {
	int c;
	int size;
	
	// check signature
	if (fgetc(f) != 'P' || fgetc(f) != '6')
		return -1;

	// read width
	while (skip_whitespaces(f) || skip_comment(f));
	if (!read_int(f, width)) return -2;
	
	// read height
	while (skip_whitespaces(f) || skip_comment(f));
	if (!read_int(f, height)) return -3;

	// read maxval
	while (skip_whitespaces(f) || skip_comment(f));
	if (!read_int(f, maxval)) return -4;

	c = fgetc(f);
	if (!(c == '\n' || c == '\t' || c == '\r' || c == ' ')) return -5; 
	if (*maxval > 65535) return -6;

	// data is not NULL, try read image pixels
	if (data) {
		if (*maxval < 256)
			size = *width * *height * 3;
		else
			size = *width * *height * 6;
		
		*data = (uint8_t*)malloc(size);
		
		if (!*data)
			return -7;
		else
			if (!fread(*data, size, 1, f)) {
				free(*data);
				return -8;
			}
	}
	return 0;
}

// eof

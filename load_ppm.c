/*
	$Date: 2007-06-30 00:06:34 $, $Revision: 1.3 $
	
	Simple PPM files (24bpp) loader/identify.
	
	Author: Wojciech Mu³a
	e-mail: wojciech_mula@poczta.onet.pl
	www:    http://www.mula.w.pl
	
	License: public domain
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

char __ppm_read_int(FILE* f, int* v) {
	int value = 0;	// readed value
	int c;			// buffer
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

char __ppm_skip_whitespaces(FILE* f) {
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

char __ppm_skip_comment(FILE* f) {
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

/*
	Load/identify PPM (i.e. 24bpp RGB).

	Input:
		f      - file

	Output:
		width,
		height - image dimensions
		maxval - maximal value of sample (usually 255)
		data   - pointer to image pixels (if NULL image pixels
		         are not loaded, no memory is allocated)
				 user have to free data

	Returns:
		status, 0 if succed, < 0 -- some error occured
*/
int load_ppm(FILE* f, int* width, int* height, int* maxval, uint8_t** data) {
	int c;
	int size;
	
	// check signature
	if (fgetc(f) != 'P' || fgetc(f) != '6')
		return -1;

	// read width
	while (__ppm_skip_whitespaces(f) || __ppm_skip_comment(f));
	if (!__ppm_read_int(f, width)) return -2;
	
	// read height
	while (__ppm_skip_whitespaces(f) || __ppm_skip_comment(f));
	if (!__ppm_read_int(f, height)) return -3;

	// read maxval
	while (__ppm_skip_whitespaces(f) || __ppm_skip_comment(f));
	if (!__ppm_read_int(f, maxval)) return -4;

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

#ifdef TEST_PPM
int main(int argc, char* argv[]) {
	int i;
	int width, height, maxval;
	int result;
	uint8_t* img;
	FILE* file;
	
	if (argc == 1)
		puts("usage: program [PPM files]");
	else
		for (i=1; i < argc; i++) {
			file = fopen(argv[i], "rb");
			if (!file) {
				fprintf(stderr, "Can't open for read '%s'\n", argv[i]);
				continue;
			}
			
			result = load_ppm(file, &width, &height, &maxval, &img);
			if (result < 0)
				fprintf(stderr, "Error loading file '%s': %s",
				       argv[i], PPM_errmsg[-result]);
			else {
				printf("PPM file '%s' has dimensions %dx%d, image size %d bytes (%sbpp)\n",
				       argv[i], width, height,
					   width * height * (maxval < 256 ? 1 : 2),
					   maxval < 256 ? "8" : "16");
				free(img);
			}
		}
}
#endif

// vim: ts=4 sw=4 nowrap noexpandtab

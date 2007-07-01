/*
	$Date: 2007-07-01 23:22:58 $, $Revision: 1.4 $
	
	Simple PPM files (24bpp) loader/identify.
	
	Author: Wojciech Mu³a
	e-mail: wojciech_mula@poczta.onet.pl
	www:    http://www.mula.w.pl
	
	License: public domain
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

char* PPM_errormsg[] = {
	/* 0 */ "no error",
	/* 1 */ "wrong signature",
	/* 2 */ "image width absent",
	/* 3 */ "image height absent",
	/* 4 */ "image maxval absent",
	/* 5 */ "no single whitespace after maxval",
	/* 6 */ "maxval larger then 65535",
	/* 7 */ "no free memory for image pixels",
	/* 8 */ "too few image pixels"
	/* 9 */ "no free memory for tmp buffer",
	/* 10 */ "can't convert, maxval > 255",
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
int ppm_identify(FILE* f, int* width, int* height, int* maxval) {
	int c;
	
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

	return 0;
}


int ppm_bytes_per_line(int img_width, int bytes_per_pixel, int unit) {
	int blocks;
	// unit in pixels
	if (unit < -1) { 
		unit   = -unit;
		blocks = (img_width + unit-1)/unit;
		return blocks * unit * bytes_per_pixel;
	}
	else
	// unit in bytes
	if (unit > 1) { 
		blocks = (img_width * bytes_per_pixel + unit-1)/unit;
		return blocks * unit;
	}
	// no rounding
	else
		return img_width * bytes_per_pixel;
}


int ppm_load(FILE* file, int* width, int* height, int* maxval, uint8_t** data) {
	int size, result;

	result = ppm_identify(file, width, height, maxval);
	if (result < 0)
		return result;
	
	// data is NULL -- do nothing
	if (data == NULL)
		return 0;

	if (*maxval < 256)
		size = *width * 3;
	else
		size = *width * 6;
		
	*data = (uint8_t*)malloc(size);
	if (!*data)
		return -7;

	if (fread((void*)*data, size, 1, file) < 1)
		return -8;
	else
		return 0;
}


int ppm_load_32bpp(FILE* file, int* width, int* height, int* maxval, uint8_t** data, int unit) {
	int size, result, y, x, bpl;
	uint8_t* tmpbuffer;
	uint8_t* dst;
	uint8_t* src;
	uint8_t  R, G, B;

	result = ppm_identify(file, width, height, maxval);
	if (result < 0)
		return result;
	
	// data is NULL -- do nothing
	if (data == NULL)
		return 0;


	if (*maxval < 256)
		bpl = ppm_bytes_per_line(*width, 4, unit);
	else
		return -10;
	
	*data = (uint8_t*)malloc(*height * bpl);
	if (!*data)
		return -7;
	
	if (*maxval < 256)
		size = *width * 3;
	else
		size = *width * 6;
	
	tmpbuffer = (uint8_t*)malloc(size);
	if (!tmpbuffer)
		return -9;
	
	// read
	dst = *data;
	for (y=0; y < *height; y++) {
		src = tmpbuffer;
		if (fread((void*)tmpbuffer, size, 1, file) < 1) {
			free(tmpbuffer);
			return -8;
		}
		for (x=0; x < *width; x++) {
			R = *src++;
			G = *src++;
			B = *src++;
			*dst++ = B;
			*dst++ = G;
			*dst++ = R;
			*dst++ = 0x00;   // pad
		}
	}

	free(tmpbuffer);
	return 0;
}


int ppm_load_16bpp(FILE* file, int* width, int* height, int* maxval, uint8_t** data, int unit) {
	int size, result, y, x, bpl;
	uint8_t* tmpbuffer;
	uint8_t* src;
	uint8_t* dst;
	uint16_t R, G, B;

	result = ppm_identify(file, width, height, maxval);
	if (result < 0)
		return result;
	
	// data is NULL -- do nothing
	if (data == NULL)
		return 0;


	if (*maxval < 256)
		bpl = ppm_bytes_per_line(*width, 2, unit);
	else
		return -10;
		
	*data = (uint8_t*)malloc(*height * bpl);
	if (!*data)
		return -7;
	
	if (*maxval < 256)
		size = *width * 3;
	else
		size = *width * 6;
	
	tmpbuffer = (uint8_t*)malloc(size);
	if (!tmpbuffer)
		return -9;
	
	// read
	dst = *data;
	for (y=0; y < *height; y++) {
		src = tmpbuffer;
		if (fread((void*)tmpbuffer, size, 1, file) < 1) {
			free(tmpbuffer);
			return -8;
		}
		for (x=0; x < *width; x++) {
			R = *src++ >> 3;
			G = *src++ >> 2;
			B = *src++ >> 3;
			*(uint16_t*)dst = (R << 11) | (G << 5) | B;
			dst += 2;
		}
	}

	free(tmpbuffer);
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
				       argv[i], PPM_errormsg[-result]);
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

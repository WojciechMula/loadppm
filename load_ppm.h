#ifndef __LOAD_PPM_H_INCLUDED__
#define __LOAD_PPM_H_INCLUDED__
/*
	$Date: 2007-07-04 20:14:45 $, $Revision: 1.1 $
	
	Simple PPM files (24bpp) loader/identify [header].
	
	Author: Wojciech Mu�a
	e-mail: wojciech_mula@poczta.onet.pl
	www:    http://www.mula.w.pl
	
	License: public domain
*/


#define PPM_maxerror 11
char* PPM_errormsg[PPM_maxerror] = {
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
	identify PPM (i.e. 24bpp RGB).

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
		status, 0 if succed, < 0 -- some error occured, error
		message: PPM_errormsg[-ret_val]
*/
int ppm_identify(
	FILE* f,
	int* width,
	int* height,
	int* maxval
);

// load original 24bpp image
int ppm_load(
	FILE* file,
	int* width,
	int* height,
	int* maxval,
	uint8_t** data
);

// load PPM and convert to 32bpp image
int ppm_load_32bpp(
	FILE* file, 
	int* width, 
	int* height, 
	int* maxval, 
	uint8_t** data, 
	int unit // units: greater then 0 -- in bytes
	         //        less then 0    -- in pixels
);

// load PPM and convert to 16bpp image
int ppm_load_16bpp(
	FILE* file,
	int* width, 
	int* height, 
	int* maxval, 
	uint8_t** data, 
	int unit
);

typedef enum {
	Simple,	// average of R, G, B
	Weigted	// weighted sum of R, G, B
} GrayScaleMode;


// load PPM and convert to grayscale image
int ppm_load_gray(
	FILE* file, 
	int* width, 
	int* height, 
	int* maxval, 
	uint8_t** data, 
	int unit, 
	GrayScaleMode gsm
);

#endif

// vim: ts=4 sw=4 nowrap noexpandtab

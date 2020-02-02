/*
  ==============================================================================

msproc: command line tool for working with Mid-Side audio files

Copyright (C) <2015> <Panos Kountanis>

This program is free software; you can redistribute it and/or modify it under the terms of
the GNU General Public License version 2 as published by the Free Software Foundation;

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

  ==============================================================================
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "sndfile.h"

#define LOGTEN 2.302585092994

const unsigned int BUF_LEN = 1024;
const short MAJOR_VERSION = 2;
const short MINOR_VERSION = 0;
const short PATCH_LEVEL = 0;

typedef enum action {
	DECODE = 0,
	ENCODE = 1,
} Action;

void usage()
{
	printf("\nmsproc %d.%d.%d\n\nUsage:\n\n    msproc <action> <input file> <output file>\n\n", MAJOR_VERSION, MINOR_VERSION, PATCH_LEVEL);
}

static double dbtorms(double f)
{
	if (f <= 0) {
		return 0.0F;
	}
	if (f > 485) {
		f = 485.0F;
	}
	return exp((LOGTEN * 0.05) * (f-100.));
}

static void ms_decode(SNDFILE *infile, SNDFILE *outfile, Action a)
{
	double buffer[BUF_LEN];
	sf_count_t count;
	unsigned int i;
	double mid, side;
	double scalar = 1.0F;
	if (a == ENCODE) {
		scalar = 0.5F;
	}

	while ((count = sf_read_double(infile, buffer, BUF_LEN)) > 0) {
		for (i = 0; i < count; i=i+2) {
			mid = buffer[i];
			side = buffer[i+1];

			buffer[i] = scalar * (mid + side);
			buffer[i+1] = scalar * (mid + (-1.0F * side));

		}
		sf_write_double(outfile, buffer, count);
	}
}

int main(int argc, char* argv[])
{
	SNDFILE *infile, *outfile;
	SF_INFO sfinfo;
	char *action;
	char *filepath;
	char *outfilepath;
	Action a;

	if (argc != 4) {
		printf("Wrong number of arguments\n");
		usage();
		return 1;
	} else {
		action = argv[1];
		filepath = argv[2];
		outfilepath = argv[3];
	}

	if (strcmp(action, "-d") == 0 || strcmp(action, "-e") == 0) {
		if (strcmp(action, "-d") == 0) {
			a = DECODE;
		} else {
			a = ENCODE;
		}
	} else {
		printf("Error : Unable to open input file '%s'\n", filepath);
		exit(1);
	}

	memset(&sfinfo, 0, sizeof (sfinfo));
	if ((infile = sf_open(filepath, SFM_READ, &sfinfo)) == NULL) {
		printf("Error : Unable to open input file '%s'\n", filepath);
		sf_close(infile);
		exit(1);
	};

	if (sfinfo.channels != 2) {
		printf("Input file '%s' not a 2-channel file. Exiting.\n", filepath);
		sf_close(infile);
		exit(1);
	}

	sfinfo.channels = 2;

	if ((outfile = sf_open(outfilepath, SFM_WRITE, &sfinfo)) == NULL) {
		printf("Error : Unable to open output file '%s'\n", outfilepath) ;
		sf_close(infile);
		exit(1);
	}

	ms_decode(infile, outfile, a);

	sf_close(infile);
	sf_close(outfile);

	return 0;
}

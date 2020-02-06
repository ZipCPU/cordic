////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	main.cpp
//
// Project:	A series of CORDIC related projects
//
// Purpose:	This is the main() c++ file for the cordic core generator.
//		It's primary purpose is to handle argument processing, and
//	call the specific cordic processing engine.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017-2020, Gisselquist Technology, LLC
//
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  (It's in the $(ROOT)/doc directory.  Run make with no
// target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
//
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "cordiclib.h"
#include "topolar.h"
#include "seqpolar.h"
#include "basiccordic.h"
#include "seqcordic.h"
#include "sintable.h"
#include "quadtbl.h"

void	usage(void) {
	fprintf(stderr,
"USAGE: gencordic [-ahrv] [-f <fname>] [-i <iw>] [-o <ow>]\n"
"\t\t[-n <stages>] [-p <phasebits>] [-t <type-of-cordic>] [-x <xtrabits>]\n"
"\n"
"\t-a\t\tCreate an auxilliary bit, useful for tracking logic through\n"
"\t\t\tthe cordic stages, and knowing when a valid output is ready.\n"
"\t-c\t\tCreate\'s a C-header file containing the numbers of bits the\n"
"\t\t\tcordic has been built for.\n"
"\t-f <fname>\tSets the output filename to <fname>\n"
"\t-h\t\tShow this message\n"
"\t-i <iw>\tSets the input bit-width\n"
"\t-n <stages>\tForces the number of cordic stages to <stages>\n"
"\t-o <ow>\tSets the output bit-width\n"
"\t-p <pw>\tSets the number of bits in the phase processor\n"
"\t-r\tCreate reset logic in the produced cordic\n"
"\t-t <type-of-cordic>\tDetermines which type of logic is created.  Two\n"
"\t\t\ttypes of cordic\'s are supported:\n"
"\t\tp2r\tPolar to rectangular.  Given a cmoplex vector, rotate it by\n"
"\t\t\tthe given number of degrees.  This is what I commonly think of\n"
"\t\t\twhen I think of a cordic.  You can use this to create sin/cos\n"
"\t\t\tfunctions, or even to multiply by a complex conjugate.\n"
"\t\tr2p\tRectangular to polar coordinate conversion\n"
"\t\tqtr\tQuarter-wave table lookup sinewave generator\n"
"\t\tqtbl\tQuadratically interpolated sinewave generator\n"
"\t\ttbl\tStraight table lookup sinewave generator\n"
"\t-v\tTurns on any verbose outputting\n"
"\t-x <xtrabits>\tUses this many extra bits in rectangular\n"
"\t\t\tvalue processing\n");
}

int	main(int argc, char **argv) {
	const int	DEFAULT_BITWIDTH = 24;
	int	nstages = -1, iw=-1, ow=-1, nxtra=2, phase_bits=-1, ww;
	const char	*fname = NULL;
	bool	with_reset = true, with_aux = true;
	bool	polar_to_rect = false, rect_to_polar = true, verbose=false,
		gen_sintable = false, gen_quarterwav = false, c_header = false,
		gen_quadtbl = false, async_reset = false,
		sequential = false;
	int	c;
	FILE	*fp, *fhp;

	while((c = getopt(argc, argv, "aAcf:hi:n:o:p:Rrt:vx:"))!=-1) {
		switch(c) {
		case 'a':
			with_aux = true;
			break;
		case 'A':
			async_reset = true;
			with_reset = true;
			break;
		case 'c':
			c_header = true;
			break;
		case 'f':
			fname = strdup(optarg);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'i':
			iw = atoi(optarg);
			break;
		case 'n':
			nstages = atoi(optarg);
			break;
		case 'o':
			ow = atoi(optarg);
			break;
		case 'p':
			phase_bits = atoi(optarg);
			break;
		case 'R':
			with_reset = false;
			break;
		case 'r':
			with_reset = true;
			break;
		case 't':
			rect_to_polar  = false;
			polar_to_rect  = false;
			gen_sintable   = false;
			gen_quarterwav = false;
			if (strcmp(optarg, "r2p")==0) {
				if (fname == NULL)
					fname = "topolar.v";
				rect_to_polar = true;
			} else if (strcmp(optarg, "sr2p")==0) {
				if (fname == NULL)
					fname = "seqpolar.v";
				rect_to_polar = true;
				sequential    = true;
			} else if (strcmp(optarg, "p2r")==0) {
				if (NULL == fname)
					fname = "basiccordic.v";
				polar_to_rect = true;
			} else if (strcmp(optarg, "sp2r")==0) {
				if (NULL == fname)
					fname = "seqcordic.v";
				polar_to_rect = true;
				sequential = true;
			} else if (strcmp(optarg, "tbl")==0) {
				if (NULL == fname)
					fname = "sintable.v";
				gen_sintable = true;
			} else if (strcmp(optarg, "qtr")==0) {
				if (NULL == fname)
					fname = "quarterwav.v";
				gen_quarterwav = true;
			} else if (strcmp(optarg, "qtbl")==0) {
				if (NULL == fname)
					fname = "quadtbl.v";
				gen_quadtbl = true;
			} else {
				fprintf(stderr, "ERR: Unsupported cordic mode, %s\n", optarg);
				exit(EXIT_FAILURE);
			} break;
		case 'v':
			verbose = true;
			break;
		case 'x':
			nxtra = atoi(optarg);
			break;
		case '?':
			if (isprint(optopt))
				fprintf(stderr, "ERR: Unknown option, -%c\n", optopt);
			else
				fprintf(stderr, "ERR: Unknown option, 0x%02x\n", optopt);
			exit(EXIT_FAILURE);
			break;
		default:
			fprintf(stderr, "ERR: Failed to process arguments\n");
			exit(EXIT_FAILURE);
		}
	}

	fhp = NULL;
	if ((NULL == fname)||(strlen(fname)==0)||(strcmp(fname, "-")==0)) {
		fp = stdout;
	} else if (NULL == (fp = fopen(fname, "w"))) {
		fprintf(stderr, "ERR: Cannot open to %s for writing\n", fname);
		perror("O/S Err:");
		exit(EXIT_FAILURE);
	} else if ((c_header)&&(!gen_sintable)&&(!gen_quarterwav)) {
		char *strp = strdup(fname);
		int	slen = strlen(fname);
		if ((slen>2)&&(strp[slen-1] == 'v')&&(strp[slen-2]=='.')) {
			strp[slen-1] = 'h';
			fhp = fopen(strp, "w");
			if (NULL == fhp)
				fprintf(stderr, "WARNING: Could not open %s\n", strp);
		} free(strp);
	}

	if (polar_to_rect) {
		if ((iw < 0)&&(ow > 0))
			iw = ow;
		if (ow < 0)
			ow = iw;
		if ((iw < 0)||(ow < 0)) {
			fprintf(stderr, "WARNING: Assuming an input and output bit-width of %d bits\n", DEFAULT_BITWIDTH);
			iw = DEFAULT_BITWIDTH;
			ow = DEFAULT_BITWIDTH;
		}
		ww = (ow > iw) ? ow:iw;
		nxtra += 1;
		ww += nxtra;
		if (phase_bits < 0)
			phase_bits = calc_phase_bits(ww);
		if (nstages < 0)
			nstages = calc_stages(ww, phase_bits);

		if (verbose) {
			printf("Building a %s cordic with the following parameters:\n"
			"\tOutput file     : %s\n"
			"\tInput  bits     : %2d\n"
			"\tExtra  bits     : %2d (used in computation, dropped when done)\n"
			"\tOutput bits     : %2d\n"
			"\tPhase  bits     : %2d\n"
			"\tNumber of stages: %2d\n",
			(sequential)?"sequential":"basic",
			(fp == stdout)?"(stdout)":fname,
			iw, nxtra, ow, phase_bits, nstages);
			if ((with_reset)&&(async_reset))
				printf("\tDesign will include an async reset signal\n");
			else if (with_reset)
				printf("\tDesign will include a reset signal\n");
			if (with_aux)
				printf("\tAux bits will be added to the design\n");
		}

		if (sequential)
			seqcordic(fp, fhp, fname,
				nstages, iw, ow, nxtra, phase_bits,
				with_reset, with_aux, async_reset);
		else
			basiccordic(fp, fhp, fname,
				nstages, iw, ow, nxtra, phase_bits,
				with_reset, with_aux, async_reset);
	} if (rect_to_polar) {
		if ((iw < 0)&&(ow > 0))
			iw = ow;
		if (ow < 0)
			ow = iw;
		if ((iw < 0)||(ow < 0)) {
			fprintf(stderr, "WARNING: Assuming an input and output bit-width of %d bits\n", DEFAULT_BITWIDTH);
			iw = DEFAULT_BITWIDTH;
			ow = DEFAULT_BITWIDTH;
		} ww = (ow > iw) ? ow:iw;
		nxtra += 2;
		ww += nxtra;
		if (phase_bits < 0)
			phase_bits = calc_phase_bits(ww);
		if (nstages < 0)
			nstages = calc_stages(phase_bits);
		if (verbose) {
			printf("Building a rectangular-to-polar CORDIC converter with the\nfollowing parameters:\n"
			"\tOutput file     : %s\n"
			"\tInput  bits     : %2d\n"
			"\tExtra  bits     : %2d (used in computation, dropped when done)\n"
			"\tOutput bits     : %2d\n"
			"\tPhase  bits     : %2d\n"
			"\tNumber of stages: %2d\n",
			(fp == stdout)?"(stdout)":fname,
			iw, nxtra, ow, phase_bits, nstages);
			if (with_reset)
				printf("\tDesign will include a reset signal\n");
			if (with_aux)
				printf("\tAux bits will be added to the design\n");
		}

		if (sequential)
			seqpolar(fp, fhp, fname,
				nstages, iw, ow, nxtra, phase_bits,
				with_reset, with_aux, async_reset);
		else
			topolar(fp, fhp, fname,
				nstages, iw, ow, nxtra, phase_bits,
				with_reset, with_aux, async_reset);
	} if (gen_sintable) {
		if ((iw >= 0)&&(phase_bits < 0)) {
			phase_bits = iw;
			iw = -1;
		}
		if (iw >= 0)
			fprintf(stderr, "WARNING: Input width parameter, -i %d, ignored for sine table generation\n", iw);
		if ((phase_bits > 3)&&(ow < 0)) {
			for(int k=phase_bits-2; k<phase_bits + 3; k++) {
				int	pb;
				pb = calc_phase_bits(k);
				if (pb == phase_bits) {
					ow = k;
					break;
				}
			}
		} if (ow < 0) {
			fprintf(stderr, "WARNING: Assuming an output bit-width of %d bits\n", DEFAULT_BITWIDTH);
			ow = DEFAULT_BITWIDTH;
		} if (phase_bits < 0)
			phase_bits = calc_phase_bits(ow);
		if (verbose) {
			printf("Building a Sinewave table lookup with the following parameters:\n"
			"\tOutput file     : %s\n"
			"\tInput  bits     : %2d\n"
			"\tPhase  bits     : %2d\n"
			"\tOutput bits     : %2d\n",
			(fp == stdout)?"(stdout)":fname,
			phase_bits, phase_bits, ow);
			if ((with_reset)&&(async_reset))
				printf("\tDesign will include an async reset signal\n");
			else if (with_reset)
				printf("\tDesign will include a reset signal\n");
			if (with_aux)
				printf("\tAux bits will be added to the design\n");
		}

		sintable(fp, fname, phase_bits, ow, with_reset, with_aux, async_reset);
	} if (gen_quarterwav) {
		if ((iw >= 0)&&(phase_bits < 0)) {
			phase_bits = iw;
			iw = -1;
		}
		if (iw >= 0)
			fprintf(stderr, "WARNING: Input width parameter, -i %d, ignored for sine table generation\n", iw);
		if ((phase_bits > 3)&&(ow < 0)) {
			for(int k=phase_bits-2; k<phase_bits + 3; k++) {
				int	pb;
				pb = calc_phase_bits(k);
				if (pb == phase_bits) {
					ow = k;
					break;
				}
			}
		} if (ow < 0) {
			fprintf(stderr, "WARNING: Assuming an output bit-width of %d bits\n", DEFAULT_BITWIDTH);
			ow = DEFAULT_BITWIDTH;
		} if (phase_bits < 0)
			phase_bits = calc_phase_bits(ow);
		if (verbose) {
			printf("Building a Sinewave table lookup with the following parameters:\n"
			"\tOutput file     : %s\n"
			"\tInput  bits     : %2d\n"
			"\tPhase  bits     : %2d\n"
			"\tOutput bits     : %2d\n",
			(fp == stdout)?"(stdout)":fname,
			phase_bits, phase_bits, ow);
			if ((with_reset)&&(async_reset))
				printf("\tDesign will include an async reset signal\n");
			else if (with_reset)
				printf("\tDesign will include a reset signal\n");
			if (with_aux)
				printf("\tAux bits will be added to the design\n");
		}

		quarterwav(fp, fname, phase_bits, ow, with_reset, with_aux, async_reset);
	} if (gen_quadtbl) {
		if ((iw < 0)&&(ow > 0))
			iw = ow;
		if (ow < 0)
			ow = iw;
		if ((iw < 0)||(ow < 0)) {
			fprintf(stderr, "WARNING: Assuming an input and output bit-width of %d bits\n", DEFAULT_BITWIDTH);
			iw = DEFAULT_BITWIDTH;
			ow = DEFAULT_BITWIDTH;
		}
		ww = (ow > iw) ? ow:iw;
		nxtra += 1;
		ww += nxtra;
		if (phase_bits < 0)
			phase_bits = calc_phase_bits(ww);
		if (nstages < 0)
			nstages = calc_stages(ww, phase_bits);

		if (verbose) {
			printf("Building a quadratically interpolated table based sine-wave calculator\n"
			"\tOutput file     : %s\n"
			// "\tInput  bits     : %2d\n"
			"\tExtra  bits     : %2d (used in computation, dropped when done)\n"
			"\tOutput bits     : %2d\n"
			"\tPhase  bits     : %2d\n",
			// "\tNumber of stages: %2d\n",
			(fp == stdout)?"(stdout)":fname, // iw,
			nxtra, ow, phase_bits);
			if ((with_reset)&&(async_reset))
				printf("\tDesign will include an async reset signal\n");
			else if (with_reset)
				printf("\tDesign will include a reset signal\n");
			if (with_aux)
				printf("\tAux bits will be added to the design\n");
		}

		/*
		basiccordic(fp, fhp, fname,
			nstages, iw, ow, nxtra, phase_bits,
			with_reset, with_aux);
		*/
		quadtbl(fp, fhp, fname, phase_bits, ow, nxtra, with_reset, with_aux, async_reset);
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Filename:	bench/cpp/fftw.c
// {{{
// Project:	A series of CORDIC related projects
//
// Purpose:	To call the Fastest Fourier Transform in the West library
//		for generic FFT requests.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2017-2024, Gisselquist Technology, LLC
// {{{
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
// }}}
// License:	GPL, v3, as defined and found on www.gnu.org,
// {{{
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
// }}}
#include <math.h>
#include "fft.h"
#include <fftw3.h>
// #include <fftw.h>

#include <assert.h>

unsigned	nextlg(unsigned long vl) {
	unsigned long	r;
	static	unsigned long	lstv=-1, lstr = 0;

	if (vl == lstv) return lstr;

	assert(vl > 0);
	for(r=1; r<vl; r<<=1)
		;
	lstv = vl; lstr = r;
	return (unsigned)r;
}

void	numer_fft(double *data, unsigned nn, int isign);

#ifndef	__cplusplus
void	cfft(double *cdata, unsigned clen) {
	numer_fft(cdata, clen, -1);
}

void	icfft(double *cdata, unsigned clen) {
	numer_fft(cdata, clen, 1);
}
#endif


static	int	initialized = 0;
typedef	struct	{
	fftw_plan	ip;
	fftw_plan	fp;
	double		*buf;
} FFTWCPLAN;

FFTWCPLAN	cplans[32];

void	numer_fft(double *data, unsigned nn, int isign) {
	fftw_plan p;
	static	double	*alt = NULL;
	unsigned	i;

	if (!initialized) {
		for(i=0; i<32; i++) {
			cplans[i].ip  = NULL;
			cplans[i].fp  = NULL;
			cplans[i].buf = NULL;
		}
		initialized = 1;
	}

	{
		unsigned long	shft;
		for(shft=0, i=2; i<nn; i<<=1, shft++)
			;
		if (cplans[shft].ip == NULL) {
			cplans[shft].buf =
				(double *)fftw_malloc(sizeof(fftw_complex)*(nn<<1));
			alt = cplans[shft].buf;

			cplans[shft].ip = fftw_plan_dft_1d(nn,
				(fftw_complex *)alt, (fftw_complex *)alt,
				FFTW_BACKWARD, FFTW_MEASURE);

			cplans[shft].fp = fftw_plan_dft_1d(nn,
				(fftw_complex *)alt, (fftw_complex *)alt,
				FFTW_FORWARD, FFTW_MEASURE);
		}

		p = (isign < 0) ? cplans[shft].fp : cplans[shft].ip;
		alt = cplans[shft].buf;
	}

	for(i=0; i<(((unsigned long)nn)<<1); i++)
		alt[i] = data[i];
	fftw_execute(p);
	for(i=0; i<(((unsigned long)nn)<<1); i++)
		data[i] = alt[i];
}

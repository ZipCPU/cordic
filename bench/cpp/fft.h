////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	fft.h
//
// Project:	A series of CORDIC related projects
//
// Purpose:	To provide a wrapper around generic access to the FFTW FFT
//		routines.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017, Gisselquist Technology, LLC
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
#ifndef	FFT_H
#define	FFT_H

#include <complex>

#ifdef	__cplusplus
extern	"C"	{
// We'll provide the same basic access interfaces that Numerical recipes used.
// The actual method may, or may not therefore, be Numerical Recipes based
extern	void	numer_fft(double *data, unsigned nn, int isign);
extern	unsigned	nextlg(unsigned long vl);
}
#endif

#ifndef	M_PI
#define	M_PI	(3.1415926535897932384626433832795028841971693993751)
#endif

#ifdef	__cplusplus
typedef	std::complex<double>	COMPLEX;
inline void	cfft(double *cdata, unsigned clen)  { numer_fft(cdata, clen, -1); }
inline void	icfft(double *cdata, unsigned clen) { numer_fft(cdata, clen,  1); }
inline void	cfft(COMPLEX *cdata, unsigned clen)  { numer_fft((double *)cdata, clen, -1); }
inline void	icfft(COMPLEX *cdata, unsigned clen) { numer_fft((double *)cdata, clen,  1); }
#else
extern	void	cfft(double *cdata, unsigned clen);
extern	void	icfft(double *cdata, unsigned clen);
#endif

#endif


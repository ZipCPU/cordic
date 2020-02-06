////////////////////////////////////////////////////////////////////////////////
//
// Filename:	quadtbl_tb.cpp
//
// Project:	A series of CORDIC related projects
//
// Purpose:	A quick test bench to determine if the sine wave generator
//		based upon a table of quadratic coefficients works.
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

#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vquadtbl.h"
#include "quadtbl.h"
#include "fft.h"
#include "testb.h"

#ifndef	HAS_AUX_WIRES
#error "This test-bench depends upon the quadtbl component having\n\tbeen configured for an aux wire."
#endif

class	QUADTBL_TB : public TESTB<Vquadtbl> {
	bool		m_debug;
public:

	QUADTBL_TB(void) {
		m_debug = true;
		m_core->i_ce    = 1;
		m_core->i_phase = 0;
		m_core->i_aux   = 0;
#ifdef	HAS_RESET_WIRE
#ifdef	ASYNC_RESET
		m_core->i_areset_n = 0;
#else
		m_core->i_reset = 1;
#endif
		tick();
#endif
	}
};

const long	LGNSAMPLES=(PW>26)?26:PW;
const long	NSAMPLES=(1ul<<LGNSAMPLES);

int main(int  argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	QUADTBL_TB	*tb = new QUADTBL_TB;
	long	*pdata, *sdata, idx;
	int	shift;
	bool	failed = false;

	pdata = new long[NSAMPLES];
	sdata = new long[NSAMPLES];

	// This only works on DUT's with the aux flag turned on.
	assert(HAS_AUX);

	// tb->opentrace("quadtbl_tb.vcd");
	tb->reset();


	idx = 0;
	for(long i=0; i<NSAMPLES; i++) {
		shift = (PW-LGNSAMPLES);
		if (shift < 0) {
			long	sv = i;
			if (i & (1ul<<(-shift)))
				// Odd value, round down
				sv += (1ul<<(-shift-1))-1;
			else
				sv += (1ul<<(-shift-1));
			tb->m_core->i_phase = sv >> (-shift);
		} else
			tb->m_core->i_phase = ((long)i) << shift;
		pdata[i] = (long)tb->m_core->i_phase;
		tb->m_core->i_aux   = 1;
		tb->tick();

		if (tb->m_core->o_aux) {
			shift = (8*sizeof(sdata[0])-OW);
			// Make our values signed..
			sdata[idx] = tb->m_core->o_sin;
			sdata[idx] <<= shift;
			sdata[idx] >>= shift;
			idx++;
		}
	}

	tb->m_core->i_aux = 0;
	shift = (8*sizeof(sdata[0])-OW);
	while(tb->m_core->o_aux) {
		tb->m_core->i_aux   = 0;
		tb->tick();

		if (tb->m_core->o_aux) {
			sdata[idx]   = tb->m_core->o_sin;
			sdata[idx] <<= shift;
			sdata[idx] >>= shift;
			idx++;
			assert(idx <= NSAMPLES);
		}
	}

	FILE	*fdbg = fopen("quadtbl.32t","w");

	double	mxerr = 0.0;
	int	imxv = 0, imnv = 0;
	for(int i=0; i<NSAMPLES; i++) {
		int	odata[3];
		double	ph, scl, dsin, err;

		odata[0] = pdata[i];
		odata[1] = sdata[i];

		ph = odata[0];
		ph = ph * M_PI * 2.0 / (double)(1ul<<PW);
		scl= ((1<<(OW-1))-1);
		dsin = sin(ph) * scl;
		odata[2] = (int)dsin;

		fwrite(odata, sizeof(int), 3, fdbg);

		err = fabs(dsin-sdata[i]);
		if (err>mxerr)
			mxerr = fabs(dsin-sdata[i]);
		if (sdata[i] > imxv)
			imxv = sdata[i];
		else if (sdata[i] < imnv)
			imnv = sdata[i];
	} fclose(fdbg);

	printf("MXERR: %f (Expected %f)\n", mxerr, TBL_ERR);
	if (fabs(mxerr) > fabs(TBL_ERR) + 2.)
		failed = true;
	printf("MXVAL: 0x%08x\n", imxv);
	printf("MNVAL: 0x%08x\n", imnv);

	if (failed)
		goto test_failed;

	// Estimate the spurious free dynamic range
	if ((PW < 26)&&(NSAMPLES == (1ul << PW))) {
		typedef	std::complex<double>	COMPLEX;
		COMPLEX	*outpt;
		const	unsigned long	FFTLEN=(1ul<<PW);

		outpt = new COMPLEX[FFTLEN];

		for(unsigned k=0; k<FFTLEN; k++) {
			outpt[k].real(sdata[(k+(FFTLEN/4))&(FFTLEN-1)]);
			outpt[k].imag(sdata[k]);
		}

		// Now we need to do an FFT
		cfft((double *)outpt, (int)FFTLEN);

		double	master, spur, tmp;

		// Master is the energy in the signal of interest
		master = norm(outpt[1]);

		// SPUR is the energy in any other FFT bin output
		spur = norm(outpt[0]);
		for(unsigned k=2; k<FFTLEN; k++) {
			tmp = norm(outpt[k]);
			if (tmp > spur)
				spur = tmp;
		}

		printf("SFDR = %7.2f dBc\n",
			10*log(master / spur)/log(10.));
	} else if (PW >= 26)
		printf("Too many phase bits ... skipping SFDR calculation\n");

	printf("SUCCESS!!\n");
	exit(EXIT_SUCCESS);

test_failed:
	printf("TEST FAILURE\n");
	exit(EXIT_FAILURE);
}


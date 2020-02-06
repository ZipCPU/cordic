////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	quadtbl.cpp
//
// Project:	A series of CORDIC related projects
//
// Purpose:	
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
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "legal.h"
#include "cordiclib.h"
#include "quadtbl.h"
#include "hexfile.h"

static	const	bool	NO_QUADRATIC_COMPONENT = false;

double	sinc(double v) {
	double	x= v * M_PI;
	return sin(x) / x;
}

static	long	max_integer(const int width) {
	return (1l<<(width-1))-2l;
}

typedef	std::string	STRING;

/*
 * x = xo + dx
 * er = TBL.c[xo]+(TBL.l[xo]+TBL.q[xo] * dx) * dx - sin(2PI(xo+dx)/N)
 * der/ddx = TBL.l[xo]+2TBL.q[xo] * dx - 2PI/N cos(2PI(xo+dx)/N)
 */
double	est_max_err(double c, double l, double q, double idx, int N) {
	double	lft, rht, mid, ph;

	ph = 2.0 * M_PI * idx / (double)N;
	lft = c - sin(ph);
	ph = 2.0 * M_PI * (idx+1) / (double)N;
	rht = c + l + q - sin(ph);


	// 
	// The following is very brute force-ish.  It doesn't need to be.
	// For large N, a sine-wave is *very* well behaved.  Brent's algorithm
	// might make more sense here, *and* get us a better answer faster.
	// This just gets me by until I look brents algorithm up in a text book
	// (again).
	//
	mid = 0;
	for(int k=0; k<64; k++) {
		double	mer = 0.0, mph, mdx;

		mdx = k / 64.0;
		mph = 2.0*M_PI*(idx + mdx)/N;
		mer = c + (l + q * mdx) * mdx - sin(mph);

		if (fabs(mer) > fabs(mid))
			mid = mer;
	}

	double	er;
	er = lft;
	if (fabs(er) < fabs(rht))
		er = rht;
	if (fabs(er) < fabs(mid))
		er = mid;

	return er;
}

double	quadtbl_spur(int lgtbl) {
	double	spur_magnitude;
	spur_magnitude = pow(sinc(1.0-(1./(1<<lgtbl))),3.);
	// spur = 20.*log(spur)/log(10.0);
	return spur_magnitude;
}

int	pick_tbl_size(int ww) {
	// Spur magnitude must be less than 0.5^ww
	double	limit = pow(0.5,ww);
	int	lgtbl;

	for(lgtbl=4; lgtbl<10; lgtbl++) {
		if (quadtbl_spur(lgtbl) < limit)
			return lgtbl;
	} return 11;
}

void	build_quadtbls(const char *fname, const int lgsz, const int wid,
		int &cbits, int &lbits, int &qbits, double &tblerr) {
	int	tbl_entries = (1<<lgsz);
	long	maxv = max_integer(wid);
	double	dl = M_PI / (double)tbl_entries, dph= dl * 2.;
	// double	scl = pow(sinc(1./tbl_entries),3);
	long	*tbldata = new long[tbl_entries];
	STRING	name;

	assert(lgsz > 2);
	assert(wid > 6);

	int	ln = tbl_entries;
	double	*table  = new double[ln];
	double	*slope  = new double[ln];
	double	*dslope = new double[ln];

	// The base value, or constant term
	for(int i=0; i<ln; i++)
		table[i] = sin(dph * i + dl);

	// The slope, or linear term
	for(int i=1; i<ln-1; i++)
		slope[i] = (table[i+1]-table[i-1])/2.0;
	slope[0   ] = (table[1]-table[ln-1])/2.0;
	slope[ln-1] = (table[0]-table[ln-2])/2.0;

	// The quadratic term
	for(int i=1; i<ln-1; i++)
		dslope[i] = -(table[i]- 0.5*(table[i+1]+table[i-1]));
	dslope[0   ] = -(table[0   ]-0.5*(table[1]+table[ln-1]));
	dslope[ln-1] = -(table[ln-1]-0.5*(table[0]+table[ln-2]));

	// Oops -- gotta adjust the base term again, creating the
	// result you would get after filtering this with our
	// quadratic.
	for(int i=0; i<ln; i++)
		table[i] = 0.75 * sin(dph*i+dl)
			+ ( sin(dph*(i-1)+dl)
			+   sin(dph*(i+1)+dl))/8.0;

	// Now, shuffle this quadratic so that we can interpolate from
	// the end, rather than the middle.
	//
	// Our current values and points provide us with
	//      y(t) = a(t-del/2)^2 + b(t-del/2) + c
	//              = at^2 + (b- a*del)t+ (a del^2/4 - b del/2 + c)
	// const        double  del = 1.0 / (double)ln;
	const   double  del = 1.0;
	const   double  hlfdel = del / 2.0;
	for(int i=0; i<ln; i++)
		table[i] = dslope[i] * hlfdel * hlfdel
				- slope[i] * hlfdel + table[i];
	for(int i=0; i<ln; i++)
		slope[i] = slope[i] - del * dslope[i];


	// Next, adjust our magnitudes, so that we'll average an
	// unscaled sine wave
	double fctr = pow(1./sinc(dl),3);
	for(int i=0; i<ln; i++)	table[i]  *= fctr;
	for(int i=0; i<ln; i++)	slope[i]  *= fctr;
	for(int i=0; i<ln; i++)	dslope[i] *= fctr;

	// Double check that we are still within bounds
	double	mxtbl = 0.0, mxslope = 0.0, mxdslope = 0.0;

	for(int i=0; i<ln; i++)
		mxtbl = (mxtbl >fabs( table[i]))?mxtbl : fabs(table[i]);
	for(int i=0; i<ln; i++)
		table[i]  *= 1./mxtbl;
	for(int i=0; i<ln; i++)
		slope[i]  *= 1./mxtbl;
	for(int i=0; i<ln; i++)
		dslope[i] *= 1./mxtbl;

	double	mxerr = 0.0, err;
	for(int i=0; i<ln; i++) {
		err = est_max_err(table[i], slope[i], dslope[i], i, ln);
		if (fabs(err) > fabs(mxerr))
			mxerr = err;
	}

	printf("MXERR = %f * %ld (0x%08lx)\n", mxerr, maxv, maxv);
	mxerr *= maxv;
	printf("MXERR = %f\n", mxerr);
	tblerr = mxerr;

	mxtbl = 0.0;
	for(int i=0; i<ln; i++)
		mxtbl = (mxtbl >fabs( table[i]))?mxtbl : fabs(table[i]);
	for(int i=0; i<ln; i++) {
		mxslope= (mxslope>fabs( slope[i]))?mxslope :fabs(slope[i]);
		mxdslope=(mxdslope>fabs(dslope[i]))?mxdslope:fabs(dslope[i]);
	}

	printf("MXVLS - TABLE:  %f -> 0x%lx\n", mxtbl, (long)(mxtbl * maxv));
	printf("MXVLS - SLOPE:  %f -> 0x%lx\n", mxslope,(long)(mxslope*maxv));
	printf("MXVLS - DSLOPE: %f -> 0x%lx\n", mxdslope,(long)(mxdslope*maxv));

	cbits   = wid + (int)ceil( log(mxtbl      )/log(2.0));
	lbits   = wid + (int)ceil(-log(1./mxslope )/log(2.0));
	qbits   = wid + (int)ceil(-log(1./mxdslope)/log(2.0));

	printf("%d WID := CBITS:LBITS:QBITS = %d:%d:%d\n", wid, cbits, lbits, qbits);
	// Double check that we are still within bounds
	for(int i=0; i<ln; i++) {
		assert(fabs(table[i])  <= (1<<(cbits-wid)));
		assert(fabs(slope[i])  <= pow(2.,(lbits-wid)));
		assert(fabs(dslope[i]) <= pow(2.,(qbits-wid)));
	}

	for(int k=0; k<tbl_entries; k++)
		tbldata[k] = (long)(maxv * table[k]);

	name = STRING(fname) + STRING("_ctbl");
	hextable(name.c_str(), lgsz, cbits, tbldata);

	for(int k=0; k<tbl_entries; k++)
		tbldata[k] = (long)(maxv * slope[k]);

	name = STRING(fname) + STRING("_ltbl");
	hextable(name.c_str(), lgsz, lbits, tbldata);

	for(int k=0; k<tbl_entries; k++)
		tbldata[k] = (long)(maxv * dslope[k]);

	name = STRING(fname) + STRING("_qtbl");
	hextable(name.c_str(), lgsz, qbits, tbldata);

	delete[] table;
	delete[] slope;
	delete[] dslope;
	delete[] tbldata;
}

void	quadtbl(FILE *fp, FILE *fhp, const char *fname, int phase_bits, int ow,
		int nxtra, bool with_reset, bool with_aux, bool async_reset) {
	const	char	*name;
	char	*noext;
	int	lgtbl = pick_tbl_size(ow+nxtra);

	int	cbits, lbits, qbits, dxbits = phase_bits-lgtbl+1;
	int	ww = ow + nxtra;
	double	tblerr;

	assert(nxtra >= 0);
	assert(fp);
	assert(phase_bits>4);
	assert(phase_bits>lgtbl);
	assert(fname);


	name = modulename(fname);
	noext = strdup(fname);
	{
		char *ptr;
		if (NULL != (ptr = strrchr(noext, '.')))
			*ptr = '\0';
	}

	lgtbl=3;
	do {
		lgtbl++;
		build_quadtbls(noext, lgtbl, ow+nxtra, cbits, lbits, qbits, tblerr);
	} while((fabs(tblerr) > 1.0)&&(lgtbl < 20));

	printf("Rpt-Err: %f\n", tblerr);
	const	char PURPOSE[] =
	"This is a sine-wave table lookup algorithm, coupled with a\n"
	"//\t\tquadratic interpolation of the result.  It's purpose is both\n"
	"//\t to trade off logic, as well as to lower the phase noise associated\n"
	"//\twith any phase truncation.",
		HPURPOSE[] =
	"This .h file notes the default parameter values from\n"
	"//\t\twithin the generated file.  It is used to communicate\n"
	"//\tinformation about the design to the bench testing code.";

	legal(fp, fname, PROJECT, PURPOSE);
	if (nxtra < 2)
		nxtra = 2;
	assert(phase_bits >= 3);

	if (ww < ow)
		ww = ow;
	ww += nxtra;

	std::string	resetw = (!with_reset) ? ""
			: (async_reset) ? "i_areset_n" : "i_reset";
	std::string	always_reset;
	if ((with_reset)&&(async_reset))
		always_reset = "\talways @(posedge i_clk, negedge i_areset_n)\n"
			"\tif (!i_areset_n)\n";
	else if (with_reset)
		always_reset = "\talways @(posedge i_clk)\n"
			"\tif (i_reset)\n";
	else
		always_reset = "\talways @(posedge i_clk)\n\t";

	fprintf(fp, "`default_nettype\tnone\n//\n");
	fprintf(fp,
		"module	%s(i_clk, %s%si_ce, %si_phase, o_sin%s);\n"
		"\tlocalparam\tPW=%2d,\t// Bits in our phase variable\n"
		"\t\t\tOW=%2d,  // The number of output bits to produce\n"
		"\t\t\tXTRA=%2d;// Extra bits for internal precision\n"
		"\tinput\twire\t\t\t\ti_clk, %s%si_ce%s;\n"
		"\t//\n"
		"\tinput\twire\tsigned\t[(PW-1):0]\ti_phase;\n"
		"\toutput\treg\tsigned\t[(OW-1):0]\to_sin;\n",
		name, resetw.c_str(), (with_reset)?", ":"",
		(with_aux)?" i_aux,":"",
		(with_aux)?", o_aux":"",
		phase_bits, ow, nxtra,
		resetw.c_str(), (with_reset)?", ":"",
		(with_aux)?", i_aux":"");


	if (with_aux)
		fprintf(fp, "\toutput\treg\t\t\t\to_aux;\n\n");

	fprintf(fp,
	"\tlocalparam\tLGTBL=%d,\n"
			"\t\t\tDXBITS  = (PW-LGTBL)+1,  // %d\n"
			"\t\t\tTBLENTRIES = (1<<LGTBL), // %d\n"
			"\t\t\tQBITS   = %d,\n"
			"\t\t\tLBITS   = %d,\n"
			"\t\t\tCBITS   = %d,\n"
			"\t\t\tWW      = (OW+XTRA), // Working width\n"
			"\t\t\tNSTAGES = %d; // Hard-coded to the algorithm\n\n",
			lgtbl, dxbits, (1<<lgtbl), qbits, lbits, cbits,
			(NO_QUADRATIC_COMPONENT)?4:6);


	fprintf(fp,
	"\t//\n"
	"\t// Space for our coefficients, and their copies as we work through\n"
	"\t// our processing stages\n");

	if (NO_QUADRATIC_COMPONENT) {
		fprintf(fp,
		"\treg\tsigned\t[(CBITS-1):0]\tcv, cv_1;\n"
		"\treg\tsigned\t[(LBITS-1):0]\tlv;\n"
		"\treg\tsigned\t[(DXBITS-1):0]\tdx;\n\n");
	} else {
		fprintf(fp,
		"\treg\tsigned\t[(CBITS-1):0]\tcv,\n"
					"\t\t\t\t\tcv_1, cv_2, cv_3;\n"
		"\treg\tsigned\t[(LBITS-1):0]\tlv, lv_1;\n"
		"\treg\tsigned\t[(QBITS-1):0]\tqv;\n"
		"\treg\tsigned\t[(DXBITS-1):0]\tdx, dx_1, dx_2;\n\n");
	}

	fprintf(fp, "\t//\n\t//\n");
	if (!NO_QUADRATIC_COMPONENT)
		fprintf(fp,
		"\treg\tsigned\t[(QBITS+DXBITS-1):0]	qprod; // [%d:%d]\n",
				qbits+dxbits-1, 0);
	if (with_aux)
		fprintf(fp,
		"\treg\t\t[(NSTAGES-1):0]\t\taux;\n");

	if (!NO_QUADRATIC_COMPONENT)
		fprintf(fp,
		"\treg\tsigned\t[(LBITS-1):0]\t\tlsum;\n");
	fprintf(fp,
		"\treg\tsigned\t[(LBITS+DXBITS-1):0]\tlprod;\n");
	if (!NO_QUADRATIC_COMPONENT)
		fprintf(fp,
		"\twire\t\t[(LBITS-1):0]\t\tw_qprod;\n");
	fprintf(fp,
	"\treg	signed	[(CBITS-1):0]		r_value; // %d bits\n"
	"\twire	signed	[(CBITS-1):0]		w_lprod;\n\n",
			cbits);

	fprintf(fp,
	"\t// Coefficient tables:\n"
	"\t//\tConstant, Linear, and Quadratic\n"
	"\treg	[(CBITS-1):0]	ctbl [0:(TBLENTRIES-1)]; //=(0...2^(OX)-1)/2^32\n"
	"\treg	[(LBITS-1):0]	ltbl [0:(TBLENTRIES-1)]; // %d x %d\n",
		lbits, (1<<lgtbl));

	if (!NO_QUADRATIC_COMPONENT)
		fprintf(fp,
	"\treg	[(QBITS-1):0]	qtbl [0:(TBLENTRIES-1)]; // %d x %d\n\n",
		qbits, (1<<lgtbl));

	fprintf(fp,
	"\tinitial begin\n"
	"\t\t$readmemh(\"%s_ctbl.hex\", ctbl);\n"
	"\t\t$readmemh(\"%s_ltbl.hex\", ltbl);\n", name, name);
	if (!NO_QUADRATIC_COMPONENT)
		fprintf(fp,
		"\t\t$readmemh(\"%s_qtbl.hex\", qtbl);\n", name);
	fprintf(fp,
		"\tend\n\n");

	if (with_aux) {
		fprintf(fp,
		"\tinitial	aux = 0;\n");

		fprintf(fp, "%s", always_reset.c_str());

		if (with_reset)
			fprintf(fp,
			"\t\taux <= 0;\n"
			"\telse ");
	
		fprintf(fp,
		"if (i_ce)\n"
			"\t\t\taux <= { aux[(NSTAGES-2):0], i_aux };\n"
			"\tassign	o_aux = aux[(NSTAGES-1)];\n\n");
	}

	fprintf(fp,
	"\t////////////////////////////////////////////////////////////////////////\n"
	"\t//\n"
	"\t//\n"
	"\t// Clock 1\n"
	"\t//	1. Operate on the incoming bits--this is the only stage\n"
	"\t//	   that does so\n"
	"\t//	2. Read our coefficients from the table\n"
	"\t//	3. Store dx, the difference between the table value and the\n"
	"\t//		actually requested phase, for later processing\n"
	"\t//\n"
	"\t//\n");

	if (!NO_QUADRATIC_COMPONENT)
		fprintf(fp, "\tinitial\tqv = 0;\n");
	fprintf(fp,
		"\tinitial\tlv = 0;\n"
		"\tinitial\tcv = 0;\n"
		"\tinitial\tdx = 0;\n");
	fprintf(fp, "%s", always_reset.c_str());

	if (with_reset) {
		fprintf(fp, "\tbegin\n");
		if (!NO_QUADRATIC_COMPONENT)
			fprintf(fp, "\t\tqv <= 0;\n");
		else
			fprintf(fp, "\t\t// No quadratic coefficient\n");
		fprintf(fp,
			"\t\tlv <= 0;\n"
			"\t\tcv <= 0;\n"
			"\t\tdx <= 0;\n"
			"\tend else ");
	}

	fprintf(fp,
	"if (i_ce)\n"
	"\tbegin\n");
	if (!NO_QUADRATIC_COMPONENT)
		fprintf(fp,"\t\tqv <= qtbl[i_phase[(PW-1):(DXBITS-1)]];\n");
	else
		fprintf(fp,"\t\t// This build has no quadratic component\n");
	fprintf(fp,
	"\t\tlv <= ltbl[i_phase[(PW-1):(DXBITS-1)]];\n"
	"\t\tcv <= ctbl[i_phase[(PW-1):(DXBITS-1)]];\n"
	"\t\tdx <= { 1'b0, i_phase[(DXBITS-2):0] };	// * 2^(-PW)\n"
	"\tend\n\n");

	fprintf(fp,
	"\t//\n"
	"\t// Here's our formula:\n"
	"\t//\n");
	if (NO_QUADRATIC_COMPONENT)
		fprintf(fp, "\t//	 Out = (     L)*DX+C\n");
	else
		fprintf(fp, "\t//	 Out = (Q*DX+L)*DX+C\n");
	fprintf(fp,
	"\t//\n"
	"\t// A basic %s interpolant.  All of the smarts are found within\n"
	"\t// the %sL, and C values.\n\n",
		(NO_QUADRATIC_COMPONENT) ? "linear":"quadratic",
		(NO_QUADRATIC_COMPONENT) ? "":"Q, ");

	if (!NO_QUADRATIC_COMPONENT) {
	fprintf(fp,
	"\t////////////////////////////////////////////////////////////////////////\n"
	"\t//\n"
	"\t//\n"
	"\t// Clock 2\n"
	"\t//	1. Multiply to get the quadratic component of our design\n"
	"\t//		This is the first of two multiplies used by this\n"
	"\t//		algorithm\n"
	"\t//	2. Everything else is just copied to the next clock\n"
	"\t//\n"
	"\t//\n");

		fprintf(fp, "\talways @(posedge i_clk)\n"
		"\tif (i_ce)\n"
			"\t\tqprod <= qv * dx; // %d bits\n\n",
				qbits+dxbits);

		fprintf(fp,
		"\tinitial	cv_1 = 0;\n"
		"\tinitial	lv_1 = 0;\n"
		"\tinitial	dx_1 = 0;\n");

		fprintf(fp, "%s", always_reset.c_str());

		if (with_reset) {
			fprintf(fp,
			"\tbegin\n"
				"\t\tcv_1 <= 0;\n"
				"\t\tlv_1 <= 0;\n"
				"\t\tdx_1 <= 0;\n"
			"\tend else ");
		}

		fprintf(fp,
			"if (i_ce) begin\n"
				"\t\tcv_1 <= cv;\n"
				"\t\tlv_1 <= lv;\n"
				"\t\tdx_1 <= dx;\n"
			"\tend\n\n");
	}

	if (!NO_QUADRATIC_COMPONENT) {
		fprintf(fp,
		"\t////////////////////////////////////////////////////////////////////////\n"
		"\t//\n"
		"\t//\n"
		"\t// Clock 3\n"
		"\t//	1. Select the number of bits we want from the output\n"
		"\t//	2. Add our linear term to the result of the multiply\n"
		"\t//	3. Copy the remaining values for the next clock\n"
		"\t//\n"
		"\t//\n");

		if (lbits-qbits-1>0) {
			fprintf(fp,
		"\tassign	w_qprod[(LBITS-1):(QBITS+1)] = { (%d){qprod[(QBITS+DXBITS-1)]} };\n",
				lbits-qbits-1);
		}
		fprintf(fp,
		"\tassign\tw_qprod[QBITS:0] // %d\n"
			"\t\t\t= qprod[(QBITS+DXBITS-1):(DXBITS-1)]; // [%d:%d]\n",
			qbits+1,qbits+dxbits-1, dxbits-1);

		fprintf(fp, "\tinitial\tlsum = 0;\n");
		fprintf(fp, "%s", always_reset.c_str());
		if (with_reset)
			fprintf(fp, "\t\tlsum <= 0;\n\telse ");

		fprintf(fp,
			"if (i_ce)\n"
				"\t\tlsum <= w_qprod + lv%s; // %d bits\n\n",
				(NO_QUADRATIC_COMPONENT)?"":"_1", lbits+1);

		fprintf(fp,
			"\tinitial\tcv_2 = 0;\n"
			"\tinitial\tdx_2 = 0;\n");
		fprintf(fp, "%s", always_reset.c_str());
		fprintf(fp,
			"\tbegin\n"
			"\t\tcv_2 <= 0;\n"
			"\t\tdx_2 <= 0;\n"
			"\tend else ");
		fprintf(fp,
			"if (i_ce) begin\n"
				"\t\tcv_2 <= cv_1;\n"
				"\t\tdx_2 <= dx_1;\n"
			"\tend\n\n");
	}


	fprintf(fp,
	"\t////////////////////////////////////////////////////////////////////////\n"
	"\t//\n"
	"\t//\n"
	"\t// Clock %d\n"
	"\t//	1. Our %s multiply\n"
	"\t//	2. Copy the constant coefficient value to the next clock\n"
	"\t//\n"
	"\t//\n",
		(NO_QUADRATIC_COMPONENT) ? 2 : 4,
		(NO_QUADRATIC_COMPONENT) ? "only" : "second and final");

	fprintf(fp,
	"\tinitial\tlprod = 0;\n"
	"\talways @(posedge i_clk)\n"
	"\tif (i_ce)\n"
			"\t\tlprod <= %s * dx%s; // %d bits\n\n",
			(NO_QUADRATIC_COMPONENT)?"lv":"lsum",
			(NO_QUADRATIC_COMPONENT)?"":"_2",
			lbits+dxbits+1);

	fprintf(fp, "\tinitial	cv_%d = 0;\n",
			(NO_QUADRATIC_COMPONENT)?1:3);


	fprintf(fp, "%s", always_reset.c_str());
	if (with_reset)
		fprintf(fp, "\t\tcv_%d <= 0;\n"
		"\telse ",
			(NO_QUADRATIC_COMPONENT)?1:3);
	fprintf(fp,
		"if (i_ce)\n"
			"\t\tcv_%d <= cv%s;\n\n",
			(NO_QUADRATIC_COMPONENT)?1:3,
			(NO_QUADRATIC_COMPONENT)?"":"_2");

	fprintf(fp,
	"\t////////////////////////////////////////////////////////////////////////\n"
	"\t//\n"
	"\t//\n"
	"\t// Clock %d\n"
	"\t//	1. Add the constant value to the result of the last\n"
	"\t//	   multiplication.  This will be the output of our algorithm\n"
	"\t//	2. There's nothing left to copy\n"
	"\t//\n"
	"\t//\n", NO_QUADRATIC_COMPONENT ? 3: 5);

//
// TBLSZ	LBITS
//	16	26
//	32	25
//	64	24	Too large
//
	if (cbits-lbits-1>0) {
		fprintf(fp,
	"\tassign	w_lprod[(CBITS-1):(LBITS+1)] = { (%d){lprod[(LBITS+DXBITS-1)]} };\n",
			cbits-lbits-1);
	}
	fprintf(fp,
	"\tassign	w_lprod[(LBITS):0] = lprod[(LBITS+DXBITS-1):(DXBITS-1)]; // %d bits\n",
			lbits);

	fprintf(fp,
	"\tinitial	r_value = 0;\n");
	fprintf(fp, "%s", always_reset.c_str());

	if (with_reset)
		fprintf(fp, "\t\tr_value <= 0;\n"
			"\telse ");

	fprintf(fp, "if (i_ce)\n"
			"\t\tr_value <= w_lprod + cv_%d;\n\n",
			(NO_QUADRATIC_COMPONENT)?1:3);

	fprintf(fp,
	"\t////////////////////////////////////////////////////////////////////////\n"
	"\t//\n"
	"\t//\n"
	"\t// Clock %d\n"
	"\t//	1. The last and final step is to round the output to the\n"
	"\t//	   nearest value.  This also involves dropping the extra bits\n"
	"\t//	   we've been carrying around since the last multiply.\n"
	"\t//\n"
	"\t//\n\n", NO_QUADRATIC_COMPONENT ? 4: 6);

	fprintf(fp,
	"\t// Since we won't be using all of the bits in w_value, we'll just\n"
	"\t// mark them all as unused for Verilator's linting purposes\n"
	"\t//\n");

	fprintf(fp,
	"\t// verilator lint_off UNUSED\n"
	"\treg	[(WW-1):0]	w_value;\n"
	"\talways @(*)\n"
	"\t\tif ((!r_value[WW-1])&&(&r_value[(WW-2):XTRA]))\n"
	"\t\t\tw_value = r_value;\n"
	"\t\telse if ((r_value[(WW-1):(WW-2)]==2'b11)&&(!|r_value[(WW-3):XTRA]))\n"
	"\t\t\tw_value = r_value;\n"
	"\t\telse\n"
	"\t\t\tw_value = r_value + { {(OW){1'b0}},\n"
				"\t\t\t\tr_value[(WW-OW)],\n"
				"\t\t\t\t{(WW-OW-1){!r_value[(WW-OW)]}} };\n"
	"\t// verilator lint_on  UNUSED\n\n");

	fprintf(fp,
	"\t//\n"
	"\t//\n"
	"\t// Calculate the final result\n"
	"\t//\n"
	"\tinitial	o_sin = 0;\n");

	fprintf(fp, "%s", always_reset.c_str());
	if (with_reset)
		fprintf(fp, "\t\to_sin <= 0;\n\telse ");

	fprintf(fp,
		"if (i_ce)\n"
			"\t\to_sin <= w_value[(WW-1):XTRA]; // [%d:%d]\n\n",
			ww, nxtra);

	fprintf(fp,
	"\t// Make verilator happy\n"
	"\t// verilator lint_off UNUSED\n"
	"\twire	[(2*(DXBITS)+XTRA-1):0] unused;\n"
	"\tassign	unused = {\n"
			"\t\t\tlprod[(DXBITS-1):0],\n"
			// "\t\t\tr_value[(CBITS-1):WW],\n"
			"\t\t\tr_value[(XTRA-1):0],\n");
	if (NO_QUADRATIC_COMPONENT) {
		fprintf(fp,
			"\t\t\t{ (DXBITS){1\'b0} }};\n");
	} else {
		fprintf(fp,
			"\t\t\tqprod[(DXBITS-1):0] };\n");
	}

	fprintf(fp, "\t// verilator lint_on  UNUSED\n\n");

	fprintf(fp, "endmodule\n");

	if (NULL != fhp) {
		char	*str = new char[strlen(name)+4], *ptr;
		sprintf(str, "%s.h", name);
		legal(fhp, str, PROJECT, HPURPOSE);
		ptr = str;
		while(*ptr) {
			if ('.' == *ptr)
				*ptr = '_';
			else	*ptr = toupper(*ptr);
			ptr++;
		}
		fprintf(fhp, "#ifndef	%s\n", str);
		fprintf(fhp, "#define	%s\n", str);
		fprintf(fhp, "const\tint\tOW         = %d; // bits\n", ow);
		fprintf(fhp, "const\tint\tNEXTRA     = %d; // bits\n", nxtra);
		fprintf(fhp, "const\tint\tPW         = %d; // bits\n", phase_bits);
		fprintf(fhp, "const\tlong\tTBL_LGSZ  = %d; // (Units)\n",lgtbl);
		fprintf(fhp, "const\tlong\tTBL_SZ    = %ld; // (Units)\n",(1l<<lgtbl));
		fprintf(fhp, "const\tlong\tSCALE     = %ld; // (Units)\n",
			max_integer(ow));
		fprintf(fhp, "const\tdouble\tITBL_ERR  = %.2f; // (OW Units)\n",
			tblerr);
		fprintf(fhp, "const\tdouble\tTBL_ERR   = %.16f; // (sin Units)\n",
			tblerr * pow(0.5,ow+nxtra));

		double	spur;
		spur = pow(sinc(1.0-(1./(1<<lgtbl))),3.);
		spur = 20.*log(spur)/log(10.0);
		fprintf(fhp, "const\tdouble\tSPURDB    = %6.2f; // dB\n", spur);

		/*
		fprintf(fhp, "const double\tQUANTIZATION_VARIANCE = %.16f; // (Units^2)\n",
			transform_quantization_variance(nstages,
				ww-iw, ww-ow));
		fprintf(fhp, "const double\tPHASE_VARIANCE_RAD = %.16f; // (Radians^2)\n",
			phase_variance(nstages, phase_bits));
		*/
		fprintf(fhp, "const\tbool\tHAS_RESET = %s;\n", with_reset?"true":"false");
		fprintf(fhp, "const\tbool\tHAS_AUX   = %s;\n", with_aux?"true":"false");
		if (with_reset)
			fprintf(fhp, "#define\tHAS_RESET_WIRE\n");
		if (with_aux)
			fprintf(fhp, "#define\tHAS_AUX_WIRES\n");
		fprintf(fhp, "#endif	// %s\n", str);

		delete[] str;
	}

	free(noext);
}

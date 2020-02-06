////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	topolar.cpp
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
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <string>
#include <ctype.h>
#include <assert.h>

#include "legal.h"
#include "cordiclib.h"
#include "topolar.h"

void	topolar(FILE *fp, FILE *fhp, const char *fname, int nstages, int iw, int ow,
		int nxtra, int phase_bits, bool with_reset, bool with_aux,
		bool async_reset) {
	int	working_width = iw;
	const	char	*name;
	const	char PURPOSE[] =
	"This is a rectangular to polar conversion routine based upon an\n"
	"//\t\tinternal CORDIC implementation.  Basically, the input is\n"
	"//\tprovided in i_xval and i_yval.  The internal CORDIC rotator will rotate\n"
	"//\t(i_xval, i_yval) until i_yval is approximately zero.  The resulting\n"
	"//\txvalue and phase will be placed into o_xval and o_phase respectively.",
		HPURPOSE[] =
	"This .h file notes the default parameter values from\n"
	"//\t\twithin the generated file.  It is used to communicate\n"
	"//\tinformation about the design to the bench testing code.";

	legal(fp, fname, PROJECT, PURPOSE);
	if (nxtra < 2)
		nxtra = 2;
	assert(phase_bits >= 3);

	if (working_width < ow)
		working_width = ow;
	working_width += nxtra;

	working_width += nxtra;
	name = modulename(fname);

	std::string	resetw = (!with_reset) ? ""
			: (async_reset) ? "i_areset_n, ":"i_reset, ";
	std::string	always_reset = "\talways @(posedge i_clk)\n\t";
	if ((with_reset)&&(async_reset))
		always_reset = "\talways @(posedge i_clk, negedge i_areset_n)\n"
			"\tif (!i_areset_n)\n";
	else if (with_reset)
		always_reset = "\talways @(posedge i_clk)\n"
			"\tif (i_reset)\n";

	fprintf(fp, "`default_nettype\tnone\n//\n");
	fprintf(fp,
		"module	%s(i_clk, %si_ce, i_xval, i_yval,%s\n"
		"\t\to_mag, o_phase%s);\n"
		"\tlocalparam\tIW=%2d,\t// The number of bits in our inputs\n"
		"\t\t\tOW=%2d,// The number of output bits to produce\n"
		"\t\t\tNSTAGES=%2d,\n"
		"\t\t\tXTRA=%2d,// Extra bits for internal precision\n"
		"\t\t\tWW=%2d,\t// Our working bit-width\n"
		"\t\t\tPW=%2d;\t// Bits in our phase variables\n"
		"\tinput\t\t\t\t\ti_clk, %si_ce;\n"
		"\tinput\twire\tsigned\t[(IW-1):0]\ti_xval, i_yval;\n"
		"\toutput\treg\tsigned\t[(OW-1):0]\to_mag;\n"
		"\toutput\treg\t\t[(PW-1):0]\to_phase;\n",
		name, resetw.c_str(),
		(with_aux)?" i_aux,":"", (with_aux)?", o_aux":"",
		iw, ow, nstages, nxtra, working_width, phase_bits,
		resetw.c_str());

	if (with_aux) {
		fprintf(fp,
			"\tinput\twire\t\t\t\ti_aux;\n"
			"\toutput\treg\t\t\t\to_aux;\n");
	}

	fprintf(fp,
		"\t// First step: expand our input to our working width.\n"
		"\t// This is going to involve extending our input by one\n"
		"\t// (or more) bits in addition to adding any xtra bits on\n"
		"\t// bits on the right.  The one bit extra on the left is to\n"
		"\t// allow for any accumulation due to the cordic gain\n"
		"\t// within the algorithm.\n"
		"\t// \n"
		"\twire\tsigned [(WW-1):0]\te_xval, e_yval;\n");

	if (working_width-iw > 2) {
		fprintf(fp,
			"\tassign\te_xval = { {(2){i_xval[(IW-1)]}}, i_xval, {(WW-IW-2){1'b0}} };\n"
			"\tassign\te_yval = { {(2){i_yval[(IW-1)]}}, i_yval, {(WW-IW-2){1'b0}} };\n\n");
	} else if (working_width-iw > 1) {
		fprintf(fp,
			"\tassign\te_xval = { {(2){i_xval[(IW-1)]}}, i_xval };\n"
			"\tassign\te_yval = { {(2){i_yval[(IW-1)]}}, i_yval };\n\n");
	} else {
		fprintf(fp,
			"\tassign\te_xval = { {(2){i_xval[(IW-1)]}, i_xval[(IW-1):1] };\n"
			"\tassign\te_yval = { {(2){i_yval[(IW-1)]}, i_yval[(IW-1):1] };\n\n");
	}

	fprintf(fp,
		"\t// Declare variables for all of the separate stages\n");

	fprintf(fp,
		"\treg	signed	[(WW-1):0]	xv	[0:NSTAGES];\n"
		"\treg	signed	[(WW-1):0]	yv	[0:NSTAGES];\n"
		"\treg		[(PW-1):0]	ph	[0:NSTAGES];\n\n");

	if (with_aux) {
		fprintf(fp,
"\t//\n"
"\t// Handle the auxilliary logic.\n"
"\t//\n"
"\t// The auxilliary bit is designed so that you can place a valid bit into\n"
"\t// the CORDIC function, and see when it comes out.  While the bit is\n"
"\t// allowed to be anything, the requirement of this bit is that it *must*\n"
"\t// be aligned with the output when done.  That is, if i_xval and i_yval\n"
"\t// are input together with i_aux, then when o_xval and o_yval are set\n"
"\t// to this value, o_aux *must* contain the value that was in i_aux.\n"
"\t//\n"
"\treg\t\t[(NSTAGES):0]\tax;\n"
"\n");

		fprintf(fp,
"\tinitial\tax = 0;\n");
		fprintf(fp, "%s", always_reset.c_str());

		if (with_reset)
			fprintf(fp,
"\t\tax <= 0;\n"
"\telse ");

		fprintf(fp, "if (i_ce)\n"
"\t\tax <= { ax[(NSTAGES-1):0], i_aux };\n"
"\n");
	}

	fprintf(fp,
		"\tinitial begin\n"
		"\t\txv[0] = 0;\n"
		"\t\tyv[0] = 0;\n"
		"\t\tph[0] = 0;\n"
		"\tend\n");
	fprintf(fp,
		"\t// First stage, map to within +/- 45 degrees\n"
		"%s", always_reset.c_str());
	if (with_reset)
		fprintf(fp,
			"\tbegin\n"
			"\t\txv[0] <= 0;\n"
			"\t\tyv[0] <= 0;\n"
			"\t\tph[0] <= 0;\n"
			"\tend else ");
	fprintf(fp, "if (i_ce)\n\t\t");

	fprintf(fp,
		"case({i_xval[IW-1], i_yval[IW-1]})\n");

	fprintf(fp,
		"\t\t2\'b01: begin // Rotate by -315 degrees\n"
		"\t\t\txv[0] <=  e_xval - e_yval;\n"
		"\t\t\tyv[0] <=  e_xval + e_yval;\n"
		"\t\t\tph[0] <= %d\'h%lx;\n"
		"\t\t\tend\n",
			phase_bits, (7ul << (phase_bits-3)));
	fprintf(fp,
		"\t\t2\'b10: begin // Rotate by -135 degrees\n"
		"\t\t\txv[0] <= -e_xval + e_yval;\n"
		"\t\t\tyv[0] <= -e_xval - e_yval;\n"
		"\t\t\tph[0] <= %d\'h%lx;\n"
		"\t\t\tend\n",
			phase_bits, (3ul << (phase_bits-3)));

	fprintf(fp,
		"\t\t2\'b11: begin // Rotate by -225 degrees\n"
		"\t\t\txv[0] <= -e_xval - e_yval;\n"
		"\t\t\tyv[0] <=  e_xval - e_yval;\n"
		"\t\t\tph[0] <= %d\'h%lx;\n"
		"\t\t\tend\n",
			phase_bits, (5ul << (phase_bits-3)));

	fprintf(fp,
		"\t\t// 2\'b00:\n"
		"\t\tdefault: begin // Rotate by -45 degrees\n"
		"\t\t\txv[0] <=  e_xval + e_yval;\n"
		"\t\t\tyv[0] <= -e_xval + e_yval;\n"
		"\t\t\tph[0] <= %d\'h%lx;\n"
		"\t\t\tend\n"
		"\t\tendcase\n",
			phase_bits, (1ul << (phase_bits-3)));

	cordic_angles(fp, nstages, phase_bits);

	fprintf(fp,"\n"
		"\tgenvar\ti;\n"
		"\tgenerate for(i=0; i<NSTAGES; i=i+1) begin : TOPOLARloop\n");

	fprintf(fp,
		"\t\tinitial begin\n"
		"\t\t\txv[i+1] = 0;\n"
		"\t\t\tyv[i+1] = 0;\n"
		"\t\t\tph[i+1] = 0;\n"
		"\t\tend\n");
	if ((with_reset)&&(async_reset))
		fprintf(fp,
			"\t\talways @(posedge i_clk, negedge i_areset_n)\n");
	else
		fprintf(fp,
		"\t\talways @(posedge i_clk)\n");

	fprintf(fp,
		"\t\t// Here\'s where we are going to put the actual CORDIC\n"
		"\t\t// rectangular to polar loop.  Everything up to this\n"
		"\t\t// point has simply been necessary preliminaries.\n");
	if (with_reset) {
		if (async_reset)
			fprintf(fp, "\t\tif (!i_areset_n)\n");
		else
			fprintf(fp, "\t\tif (i_reset)\n");
		fprintf(fp,
			"\t\tbegin\n"
			"\t\t\txv[i+1] <= 0;\n"
			"\t\t\tyv[i+1] <= 0;\n"
			"\t\t\tph[i+1] <= 0;\n"
			"\t\tend else if (i_ce)\n");
	} else
		fprintf(fp,
			"\t\tif (i_ce)\n");

	fprintf(fp,
		"\t\tbegin\n"
		"\t\t\tif ((cordic_angle[i] == 0)||(i >= WW))\n"
		"\t\t\tbegin // Do nothing but move our vector\n"
		"\t\t\t// forward one stage, since we have more\n"
		"\t\t\t// stages than valid data\n"
		"\t\t\t\txv[i+1] <= xv[i];\n"
		"\t\t\t\tyv[i+1] <= yv[i];\n"
		"\t\t\t\tph[i+1] <= ph[i];\n"
		"\t\t\tend else if (yv[i][(WW-1)]) // Below the axis\n"
		"\t\t\tbegin\n"
		"\t\t\t\t// If the vector is below the x-axis, rotate by\n"
		"\t\t\t\t// the CORDIC angle in a positive direction.\n"
		"\t\t\t\txv[i+1] <= xv[i] - (yv[i]>>>(i+1));\n"
		"\t\t\t\tyv[i+1] <= yv[i] + (xv[i]>>>(i+1));\n"
		"\t\t\t\tph[i+1] <= ph[i] - cordic_angle[i];\n"
		"\t\t\tend else begin\n"
		"\t\t\t\t// On the other hand, if the vector is above the\n"
		"\t\t\t\t// x-axis, then rotate in the other direction\n"
		"\t\t\t\txv[i+1] <= xv[i] + (yv[i]>>>(i+1));\n"
		"\t\t\t\tyv[i+1] <= yv[i] - (xv[i]>>>(i+1));\n"
		"\t\t\t\tph[i+1] <= ph[i] + cordic_angle[i];\n"
		"\t\t\tend\n"
		"\t\tend\n"
		"\tend endgenerate\n\n");

	if (working_width > ow+1) {
		fprintf(fp,
			"\t// Round our magnitude towards even\n"
			"\twire\t[(WW-1):0]\tpre_mag;\n\n"
			"\tassign\tpre_mag = xv[NSTAGES] + $signed({{(OW){1\'b0}},\n"
				"\t\t\t\txv[NSTAGES][(WW-OW)],\n"
				"\t\t\t\t{(WW-OW-1){!xv[NSTAGES][WW-OW]}}});\n"
			"\n");

		fprintf(fp,
			"\tinitial\to_mag   = 0;\n"
			"\tinitial\to_phase = 0;\n");
		if (with_aux)
			fprintf(fp, "\tinitial\to_aux   = 0;\n");
		fprintf(fp, "%s", always_reset.c_str());
		if (with_reset) {
			fprintf(fp,
				"\tbegin\n"
				"\t\to_mag   <= 0;\n"
				"\t\to_phase <= 0;\n");
			if (with_aux)
				fprintf(fp,
				"\t\to_aux   <= 0;\n");
			fprintf(fp, "\tend else ");
		}

		fprintf(fp, "if (i_ce)\n"
			"\tbegin\n"
			"\t\to_mag   <= pre_mag[(WW-1):(WW-OW)];\n"
			"\t\to_phase <= ph[NSTAGES];\n");
		if (with_aux)
			fprintf(fp,
			"\t\to_aux <= ax[NSTAGES];\n");
		fprintf(fp, "\tend\n\n");

		fprintf(fp, "\t// Make Verilator happy with pre_.val\n"
			"\t// verilator lint_off UNUSED\n"
			"\twire	[(WW-OW):0] unused_val;\n"
			"\tassign\tunused_val = {"
			" pre_mag[WW-1], pre_mag[(WW-OW-1):0] };\n"
			"\t// verilator lint_on UNUSED\n");
	} else {
		fprintf(fp,
			"\tinitial\to_mag   = 0;\n"
			"\tinitial\to_phase = 0;\n");
		if (with_aux)
			fprintf(fp, "\tinitial\to_aux = 0;\n");
		fprintf(fp, "%s", always_reset.c_str());

		if (with_reset) {
			fprintf(fp, "\tbegin\n"
			"\t\to_mag   <= 0;\n"
			"\t\to_phase <= 0;\n");
			if (with_aux)
				fprintf(fp, "\t\to_aux  <= 0;\n");
			fprintf(fp, "\tend else ");
		}

		fprintf(fp, "if (i_ce)\n"
			"\tbegin\t// We accumulate a bit during our processing, so shift by one\n"
			"\t\to_mag   <= xv[NSTAGES][(WW-1):(WW-OW)];\n"
			"\t\to_phase <= ph[NSTAGES];\n");
		if (with_aux)
			fprintf(fp, "\t\to_aux  <= ax[NSTAGES];\n");
		fprintf(fp, "\tend\n\n");
	}

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
		if (async_reset)
			fprintf(fhp, "#define\tASYNC_RESET\n");
		fprintf(fhp, "const int	IW = %d;\n", iw);
		fprintf(fhp, "const int	OW = %d;\n", ow);
		fprintf(fhp, "const int	NEXTRA = %d;\n", nxtra);
		fprintf(fhp, "const int	WW = %d;\n", working_width);
		fprintf(fhp, "const int	PW = %d;\n", phase_bits);
		fprintf(fhp, "const int	NSTAGES = %d;\n", nstages);
		fprintf(fhp, "const double\tQUANTIZATION_VARIANCE = %.16f; // (Units^2)\n",
			transform_quantization_variance(nstages,
				working_width-iw, working_width-ow));
		fprintf(fhp, "const double\tPHASE_VARIANCE_RAD = %.16f; // (Radians^2)\n",
			phase_variance(nstages, phase_bits));
		fprintf(fhp, "const double\tGAIN = %.16f;\n",
			cordic_gain(nstages));
		fprintf(fhp, "const bool\tHAS_RESET = %s;\n", with_reset?"true":"false");
		fprintf(fhp, "const bool\tHAS_AUX   = %s;\n", with_aux?"true":"false");
		if (with_reset)
			fprintf(fhp, "#define\tHAS_RESET_WIRE\n");
		if (with_aux)
			fprintf(fhp, "#define\tHAS_AUX_WIRES\n");
		fprintf(fhp, "#endif	// %s\n", str);

		delete[] str;
	}
}

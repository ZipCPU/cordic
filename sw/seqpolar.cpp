////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	seqpolar.cpp
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
// Copyright (C) 2018, Gisselquist Technology, LLC
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

void	seqpolar(FILE *fp, FILE *fhp, const char *fname, int nstages, int iw, int ow,
		int nxtra, int phase_bits, bool with_reset, bool with_aux,
		bool async_reset) {
	int	working_width = iw;
	const	char	*name;
	const	char PURPOSE[] =
	"This is a rectangular to polar conversion routine based upon an\n"
	"//\t\tinternal CORDIC implementation.  Basically, the input is\n"
	"//\tprovided in i_xval and i_yval.  The internal CORDIC rotator will rotate\n"
	"//\t(i_xval, i_yval) until i_yval is approximately zero.  The resulting\n"
	"//\txvalue and phase will be placed into o_xval and o_phase respectively.\n"
	"//\n"
	"//\tThis particular version of the polar to rectangular CORDIC converter\n"
	"//\tconverter processes a somple one at a time.  It is completely\n"
	"//\tsequential, not parallel at all.\n//",
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
		"module	%s(i_clk, %si_stb, i_xval, i_yval, %so_busy,\n"
		"\t\to_done, o_mag, o_phase%s);\n"
		"\tlocalparam\tIW=%2d,\t// The number of bits in our inputs\n"
		"\t\t\tOW=%2d,// The number of output bits to produce\n"
		"\t\t\tNSTAGES=%2d,\n"
		"\t\t\tXTRA=%2d,// Extra bits for internal precision\n"
		"\t\t\tWW=%2d,\t// Our working bit-width\n"
		"\t\t\tPW=%2d;\t// Bits in our phase variables\n"
		"\tinput\t\t\t\t\ti_clk, %si_stb;\n"
		"\tinput\twire\tsigned\t[(IW-1):0]\ti_xval, i_yval;\n"
		"\toutput\twire\t\t\t\to_busy;\n"
		"\toutput\treg\t\t\t\to_done;\n"
		"\toutput\treg\tsigned\t[(OW-1):0]\to_mag;\n"
		"\toutput\treg\t\t[(PW-1):0]\to_phase;\n",
		name, resetw.c_str(),
		(with_aux)?"i_aux, ":"", (with_aux)?", o_aux":"",
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
		"\treg	signed	[(WW-1):0]	xv, yv, prex, prey;\n"
		"\treg		[(PW-1):0]	ph, preph;\n\n");

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
"\treg\t\taux;\n"
"\n");

		fprintf(fp, "%s", always_reset.c_str());

		if (with_reset)
			fprintf(fp,
"\t\taux <= 0;\n"
"\telse ");

		fprintf(fp, "if ((i_stb)&&(!o_busy))\n"
"\t\taux <= i_aux;\n"
"\n");
	}

	fprintf(fp,
		"\t// First stage, map to within +/- 45 degrees\n"
		"\talways @(posedge i_clk)\n");
	fprintf(fp,
		"\t\tcase({i_xval[IW-1], i_yval[IW-1]})\n");

	fprintf(fp,
		"\t\t2\'b01: begin // Rotate by -315 degrees\n"
		"\t\t\tprex <=  e_xval - e_yval;\n"
		"\t\t\tprey <=  e_xval + e_yval;\n"
		"\t\t\tpreph <= %d\'h%lx;\n"
		"\t\t\tend\n",
			phase_bits, (7ul << (phase_bits-3)));
	fprintf(fp,
		"\t\t2\'b10: begin // Rotate by -135 degrees\n"
		"\t\t\tprex <= -e_xval + e_yval;\n"
		"\t\t\tprey <= -e_xval - e_yval;\n"
		"\t\t\tpreph <= %d\'h%lx;\n"
		"\t\t\tend\n",
			phase_bits, (3ul << (phase_bits-3)));

	fprintf(fp,
		"\t\t2\'b11: begin // Rotate by -225 degrees\n"
		"\t\t\tprex <= -e_xval - e_yval;\n"
		"\t\t\tprey <=  e_xval - e_yval;\n"
		"\t\t\tpreph <= %d\'h%lx;\n"
		"\t\t\tend\n",
			phase_bits, (5ul << (phase_bits-3)));

	fprintf(fp,
		"\t\t// 2\'b00:\n"
		"\t\tdefault: begin // Rotate by -45 degrees\n"
		"\t\t\tprex <=  e_xval + e_yval;\n"
		"\t\t\tprey <= -e_xval + e_yval;\n"
		"\t\t\tpreph <= %d\'h%lx;\n"
		"\t\t\tend\n"
		"\t\tendcase\n",
			phase_bits, (1ul << (phase_bits-3)));

	cordic_angles(fp, nstages, phase_bits, true);

	fprintf(fp, "\n\treg\t\tidle, pre_valid;\n");
	fprintf(fp, "\treg\t[%d:0]\tstate;\n\n",
			nextlg((unsigned)nstages+1)-1);
	fprintf(fp, "\twire	last_state;\n");
	fprintf(fp, "\tassign	last_state = (state >= %d);\n", nstages+1);
	fprintf(fp,
		"\n\tinitial\tidle = 1\'b1;\n%s", always_reset.c_str());
	if (with_reset)
		fprintf(fp, "\t\tidle <= 1\'b1;\n\telse ");
	else
		fprintf(fp, "\t");
	fprintf(fp, "if (i_stb)\n"
			"\t\tidle <= 1\'b0;\n"
			"\telse if (last_state)\n"
			"\t\tidle <= 1\'b1;\n\n");

	fprintf(fp,
		"\tinitial\tpre_valid = 1\'b0;\n%s", always_reset.c_str());
	if (with_reset)
		fprintf(fp, "\t\tpre_valid <= 1\'b0;\n\telse\n");
	fprintf(fp, "\t\tpre_valid <= (i_stb)&&(idle);\n\n");


	fprintf(fp,
		"\tinitial\tstate = 0;\n%s", always_reset.c_str());
	if (with_reset)
		fprintf(fp, "\t\tstate <= 0;\n\telse ");
	else
		fprintf(fp, "\t");
	fprintf(fp, "if (idle)\n"
			"\t\tstate <= 0;\n"
		"\telse if (last_state)\n"
			"\t\tstate <= 0;\n"
		"\telse\n"
			"\t\tstate <= state + 1;\n\n");


	fprintf(fp,
		"\talways @(posedge i_clk)\n"
		"\t\tcangle <= cordic_angle[state[%d:0]];\n\n",
			nextlg((unsigned)nstages)-1);

	fprintf(fp,
		"\t// Here\'s where we are going to put the actual CORDIC\n"
		"\t// rectangular to polar loop.  Everything up to this\n"
		"\t// point has simply been necessary preliminaries.\n");

	fprintf(fp, "\talways @(posedge i_clk)\n"
		"\tif (pre_valid)\n"
		"\tbegin\n"
		"\t\txv <= prex;\n"
		"\t\tyv <= prey;\n"
		"\t\tph <= preph;\n"
		"\tend else if (yv[(WW-1)]) // Below the axis\n"
		"\tbegin\n"
		"\t\t// If the vector is below the x-axis, rotate by\n"
		"\t\t// the CORDIC angle in a positive direction.\n"
		"\t\txv <= xv - (yv>>>state);\n"
		"\t\tyv <= yv + (xv>>>state);\n"
		"\t\tph <= ph - cangle;\n"
		"\tend else begin\n"
		"\t\t// On the other hand, if the vector is above the\n"
		"\t\t// x-axis, then rotate in the other direction\n"
		"\t\txv <= xv + (yv>>>state);\n"
		"\t\tyv <= yv - (xv>>>state);\n"
		"\t\tph <= ph + cangle;\n"
		"\tend\n\n");

	fprintf(fp, "%s", always_reset.c_str());
	if (with_reset)
		fprintf(fp, "\t\to_done <= 1\'b0;\n\telse\n");
	fprintf(fp, "\t\to_done <= (last_state);\n\n");

	if (working_width > ow+1) {
		fprintf(fp,
			"\t// Round our magnitude towards even\n"
			"\twire\t[(WW-1):0]\tfinal_mag;\n\n"
			"\tassign\tfinal_mag = xv + $signed({{(OW){1\'b0}},\n"
				"\t\t\t\txv[(WW-OW)],\n"
				"\t\t\t\t{(WW-OW-1){!xv[WW-OW]}}});\n"
			"\n");


		fprintf(fp, "\talways @(posedge i_clk)\n");
		fprintf(fp,
			"\tif (last_state)\n"
			"\tbegin\n"
			"\t\to_mag   <= final_mag[(WW-1):(WW-OW)];\n");
	} else {
		fprintf(fp, "\talways @(posedge i_clk)\n");
		fprintf(fp,
			"\tif (last_state)\n"
			"\tbegin\t// We accumulate a bit during our processing, so shift by one\n"
			"\t\to_mag   <= xv[(WW-1):(WW-OW)];\n");
	}

	fprintf(fp, "\t\to_phase <= ph;\n");
	if (with_aux)
		fprintf(fp,
			"\t\to_aux <= aux;\n");
	fprintf(fp, "\tend\n\n");

	fprintf(fp, "\tassign\to_busy = !idle;\n\n");

	if (working_width > ow+1) {
		fprintf(fp, "\t// Make Verilator happy with pre_.val\n"
			"\t// verilator lint_off UNUSED\n"
			"\twire	[(WW-OW):0] unused_val;\n"
			"\tassign\tunused_val = {"
			" final_mag[WW-1], final_mag[(WW-OW-1):0] };\n"
			"\t// verilator lint_on UNUSED\n");
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
		fprintf(fhp, "#ifndef\t%s\n", str);
		fprintf(fhp, "#define\t%s\n", str);
		if (async_reset)
			fprintf(fhp, "#define\tASYNC_RESET\n");

		fprintf(fhp, "#ifdef\tCLOCKS_PER_OUTPUT\n");
		fprintf(fhp, "#undef\tCLOCKS_PER_OUTPUT\n");
		fprintf(fhp, "#endif\t// CLOCKS_PER_OUTPUT\n");
		fprintf(fhp, "#define\tCLOCKS_PER_OUTPUT\t%d\n", nstages+3);

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

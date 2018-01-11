////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	basiccordic.cpp
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
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <string>
#include <ctype.h>
#include <assert.h>

#include "legal.h"
#include "cordiclib.h"
#include "basiccordic.h"

void	basiccordic(FILE *fp, FILE *fhp, const char *fname,
		int nstages, int iw, int ow, int nxtra,
		int phase_bits,
		bool with_reset, bool with_aux, bool async_reset) {
	int	working_width = iw;
	const	char *name;
	const	char PURPOSE[] =
	"This file executes a vector rotation on the values\n"
	"//\t\t(i_xval, i_yval).  This vector is rotated left by\n"
	"//\ti_phase.  i_phase is given by the angle, in radians, multiplied by\n"
	"//\t2^32/(2pi).  In that fashion, a two pi value is zero just as a zero\n"
	"//\tangle is zero.",
		HPURPOSE[] =
	"This .h file notes the default parameter values from\n"
	"//\t\twithin the generated file.  It is used to communicate\n"
	"//\tinformation about the design to the bench testing code.";
	legal(fp, fname, PROJECT, PURPOSE);
	if (nxtra < 1)
		nxtra = 1;
	assert(phase_bits >= 3);

	if (working_width < ow)
		working_width = ow;
	working_width += nxtra;

	std::string	resetw = (!with_reset)?""
			: ((async_reset)?"i_areset_n" : "i_reset");
	std::string	always_reset = "\talways @(posedge i_clk)\n\t";
	if ((with_reset)&&(async_reset))
		always_reset = "\talways @(posedge i_clk, negedge i_areset_n)\n"
				"\tif (!i_areset_n)\n";
	else if (with_reset)
		always_reset = "\talways @(posedge i_clk)\n"
				"\tif (i_reset)\n";

	name = modulename(fname);

	fprintf(fp, "`default_nettype\tnone\n//\n");
	fprintf(fp,
		"module	%s(i_clk, %s%si_ce, i_xval, i_yval, i_phase,%s\n"
		"\t\to_xval, o_yval%s);\n"
		"\tlocalparam\tIW=%2d,\t// The number of bits in our inputs\n"
		"\t\t\tOW=%2d,\t// The number of output bits to produce\n"
		"\t\t\tNSTAGES=%2d,\n"
		"\t\t\tXTRA=%2d,// Extra bits for internal precision\n"
		"\t\t\tWW=%2d,\t// Our working bit-width\n"
		"\t\t\tPW=%2d;\t// Bits in our phase variables\n"
		"\tinput\twire\t\t\t\ti_clk, %s%si_ce;\n"
		"\tinput\twire\tsigned\t[(IW-1):0]\t\ti_xval, i_yval;\n"
		"\tinput\twire\t\t[(PW-1):0]\t\t\ti_phase;\n"
		"\toutput\treg\tsigned\t[(OW-1):0]\to_xval, o_yval;\n",
		name, resetw.c_str(), (with_reset)?", ":"",
		(with_aux)?" i_aux,":"", (with_aux)?", o_aux":"",
		iw, ow, nstages, nxtra, working_width, phase_bits,
		resetw.c_str(), (with_reset)?", ":"");

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

	if (working_width-iw-1 > 0) {
		fprintf(fp,
			"\tassign\te_xval = { {i_xval[(IW-1)]}, i_xval, {(WW-IW-1){1'b0}} };\n"
			"\tassign\te_yval = { {i_yval[(IW-1)]}, i_yval, {(WW-IW-1){1'b0}} };\n\n");
	} else {
		fprintf(fp,
			"\tassign\te_xval = { {i_xval[(IW-1)]}, i_xval };\n"
			"\tassign\te_yval = { {i_yval[(IW-1)]}, i_yval };\n\n");
	}

	fprintf(fp,
		"\t// Declare variables for all of the separate stages\n");

	fprintf(fp,
		"\treg	signed	[(WW-1):0]	xv	[0:(NSTAGES)];\n"
		"\treg	signed	[(WW-1):0]	yv	[0:(NSTAGES)];\n"
		"\treg		[(PW-1):0]	ph	[0:(NSTAGES)];\n\n");

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

		fprintf(fp, "%s", always_reset.c_str());

		if (with_reset)
			fprintf(fp,
				"\t\tax <= {(NSTAGES+1){1'b0}};\n\telse ");
		fprintf(fp, "if (i_ce)\n"
			"\t\tax <= { ax[(NSTAGES-1):0], i_aux };\n"
			"\n");
	}

	fprintf(fp,
		"\t// First stage, get rid of all but 45 degrees\n"
		"\t//\tThe resulting phase needs to be between -45 and 45\n"
		"\t//\t\tdegrees but in units of normalized phase\n");

	fprintf(fp, "%s", always_reset.c_str());

	if (with_reset)
		fprintf(fp,
			"\tbegin\n"
			"\t\txv[0] <= 0;\n"
			"\t\tyv[0] <= 0;\n"
			"\t\tph[0] <= 0;\n"
			"\tend else ");

	fprintf(fp, "if (i_ce)\n"
		"\tbegin\n"
		"\t\t// Walk through all possible quick phase shifts necessary\n"
		"\t\t// to constrain the input to within +/- 45 degrees.\n"
		"\t\tcase(i_phase[(PW-1):(PW-3)])\n");

	fprintf(fp,
		"\t\t3'b000: begin	// 0 .. 45, No change\n"
		"\t\t\txv[0] <= e_xval;\n"
		"\t\t\tyv[0] <= e_yval;\n"
		"\t\t\tph[0] <= i_phase;\n"
		"\t\t\tend\n");

	fprintf(fp,
		"\t\t3'b001: begin	// 45 .. 90\n"
		"\t\t\txv[0] <= -e_yval;\n"
		"\t\t\tyv[0] <= e_xval;\n"
		"\t\t\tph[0] <= i_phase - %d\'h%lx;\n"
		"\t\t\tend\n",
			phase_bits, (1ul << (phase_bits-2)));

	fprintf(fp,
		"\t\t3'b010: begin	// 90 .. 135\n"
		"\t\t\txv[0] <= -e_yval;\n"
		"\t\t\tyv[0] <= e_xval;\n"
		"\t\t\tph[0] <= i_phase - %d\'h%lx;\n"
		"\t\t\tend\n",
			phase_bits, (1ul << (phase_bits-2)));

	fprintf(fp,
		"\t\t3'b011: begin	// 135 .. 180\n"
		"\t\t\txv[0] <= -e_xval;\n"
		"\t\t\tyv[0] <= -e_yval;\n"
		"\t\t\tph[0] <= i_phase - %d\'h%lx;\n"
		"\t\t\tend\n",
			phase_bits, (2ul << (phase_bits-2)));

	fprintf(fp,
		"\t\t3'b100: begin	// 180 .. 225\n"
		"\t\t\txv[0] <= -e_xval;\n"
		"\t\t\tyv[0] <= -e_yval;\n"
		"\t\t\tph[0] <= i_phase - %d\'h%lx;\n"
		"\t\t\tend\n",
			phase_bits, (2ul << (phase_bits-2)));

	fprintf(fp,
		"\t\t3'b101: begin	// 225 .. 270\n"
		"\t\t\txv[0] <= e_yval;\n"
		"\t\t\tyv[0] <= -e_xval;\n"
		"\t\t\tph[0] <= i_phase - %d\'h%lx;\n"
		"\t\t\tend\n",
		phase_bits, (3ul << (phase_bits-2)));

	fprintf(fp,
		"\t\t3'b110: begin	// 270 .. 315\n"
		"\t\t\txv[0] <= e_yval;\n"
		"\t\t\tyv[0] <= -e_xval;\n"
		"\t\t\tph[0] <= i_phase - %d\'h%lx;\n"
		"\t\t\tend\n",
		phase_bits, (3ul << (phase_bits-2)));

	fprintf(fp,
		"\t\t3'b111: begin	// 315 .. 360, No change\n"
		"\t\t\txv[0] <= e_xval;\n"
		"\t\t\tyv[0] <= e_yval;\n"
		"\t\t\tph[0] <= i_phase;\n"
		"\t\t\tend\n");

	fprintf(fp,
		"\t\tendcase\n"
		"\tend\n"
		"\n");

	cordic_angles(fp, nstages, phase_bits);

	fprintf(fp,"\n"
		"\tgenvar	i;\n"
		"\tgenerate for(i=0; i<NSTAGES; i=i+1) begin : CORDICops\n");
	if ((with_reset)&&(async_reset))
		fprintf(fp, "\t\talways @(posedge i_clk, negedge i_areset_n)\n");
	else
		fprintf(fp, "\t\talways @(posedge i_clk)\n");
	fprintf(fp,
		"\t\t// Here\'s where we are going to put the actual CORDIC\n"
		"\t\t// we\'ve been studying and discussing.  Everything up to\n"
		"\t\t// this point has simply been necessary preliminaries.\n");
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
			"\t\tend else ");
	} else
		fprintf(fp, "\t\t");

	fprintf(fp,
		"if (i_ce)\n"
		"\t\tbegin\n"
		"\t\t\tif ((cordic_angle[i] == 0)||(i >= WW))\n"
		"\t\t\tbegin // Do nothing but move our outputs\n"
		"\t\t\t// forward one stage, since we have more\n"
		"\t\t\t// stages than valid data\n"
		"\t\t\t\txv[i+1] <= xv[i];\n"
		"\t\t\t\tyv[i+1] <= yv[i];\n"
		"\t\t\t\tph[i+1] <= ph[i];\n"
		"\t\t\tend else if (ph[i][(PW-1)]) // Negative phase\n"
		"\t\t\tbegin\n"
		"\t\t\t\t// If the phase is negative, rotate by the\n"
		"\t\t\t\t// CORDIC angle in a clockwise direction.\n"
		"\t\t\t\txv[i+1] <= xv[i] + (yv[i]>>>(i+1));\n"
		"\t\t\t\tyv[i+1] <= yv[i] - (xv[i]>>>(i+1));\n"
		"\t\t\t\tph[i+1] <= ph[i] + cordic_angle[i];\n"
		"\t\t\tend else begin\n"
		"\t\t\t\t// On the other hand, if the phase is\n"
		"\t\t\t\t// positive ... rotate in the\n"
		"\t\t\t\t// counter-clockwise direction\n"
		"\t\t\t\txv[i+1] <= xv[i] - (yv[i]>>>(i+1));\n"
		"\t\t\t\tyv[i+1] <= yv[i] + (xv[i]>>>(i+1));\n"
		"\t\t\t\tph[i+1] <= ph[i] - cordic_angle[i];\n"
		"\t\t\tend\n"
		"\t\tend\n"
		"\tend endgenerate\n\n");

	if (working_width > ow+1) {
		fprintf(fp,
			"\t// Round our result towards even\n"
			"\twire\t[(WW-1):0]\tpre_xval, pre_yval;\n\n"
			"\tassign\tpre_xval = xv[NSTAGES] + $signed({{(OW){1\'b0}},\n"
				"\t\t\t\txv[NSTAGES][(WW-OW)],\n"
				"\t\t\t\t{(WW-OW-1){!xv[NSTAGES][WW-OW]}}});\n"
			"\tassign\tpre_yval = yv[NSTAGES] + $signed({{(OW){1\'b0}},\n"
				"\t\t\t\tyv[NSTAGES][(WW-OW)],\n"
				"\t\t\t\t{(WW-OW-1){!yv[NSTAGES][WW-OW]}}});\n"
			"\n");
		if ((with_reset)&&(async_reset))
			fprintf(fp, "\talways @(posedge i_clk, negedge i_areset_n)\n"
				"\tif (!i_areset_n)\n");
		else if (with_reset)
			fprintf(fp, "\talways @(posedge i_clk)\n"
				"\tif (i_reset)\n");
		else
			fprintf(fp, "\talways @(posedge i_clk)\n\t");

		if (with_reset)
			fprintf(fp, "\tbegin\n"
			"\t\to_xval <= 0;\n"
			"\t\to_yval <= 0;\n"
			"\tend else ");

		fprintf(fp,
			"if (i_ce)\n"
			"\tbegin\n"
			"\t\to_xval <= pre_xval[(WW-1):(WW-OW)];\n"
			"\t\to_yval <= pre_yval[(WW-1):(WW-OW)];\n");
		if (with_aux)
			fprintf(fp,
			"\t\to_aux <= ax[NSTAGES];\n");
		fprintf(fp, "\tend\n\n");

		fprintf(fp, "\t// Make Verilator happy with pre_.val\n"
			"\t// verilator lint_off UNUSED\n"
			"\twire	[(2*(WW-OW)-1):0] unused_val;\n"
			"\tassign\tunused_val = {\n"
			"\t\tpre_xval[(WW-OW-1):0],\n"
			"\t\tpre_yval[(WW-OW-1):0]\n"
			"\t\t};\n"
			"\t// verilator lint_on UNUSED\n");
	} else {

		if ((with_reset)&&(async_reset))
			fprintf(fp, "\talways @(posedge i_clk, negedge i_areset_n)\n"
				"\tif (!i_areset_n)\n");
		else if (with_reset)
			fprintf(fp, "\talways @(posedge i_clk)\n"
				"\tif (i_reset)\n");
		else
			fprintf(fp, "\talways @(posedge i_clk)\n\t");

		if (with_reset)
			fprintf(fp,
			"\tbegin\n"
			"\t\to_xval <= 0;\n"
			"\t\to_yval <= 0;\n"
			"\tend else ");

		fprintf(fp,
			"if (i_ce)\n"
			"\tbegin\t// We accumulate a bit during our processing, so shift by one\n"
			"\t\to_xval <= xv[NSTAGES][(WW-1):(WW-OW)];\n"
			"\t\to_yval <= yv[NSTAGES][(WW-1):(WW-OW)];\n");
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
		fprintf(fhp, "const double	QUANTIZATION_VARIANCE = %.4e; // (Units^2)\n",
			transform_quantization_variance(nstages,
				working_width-iw,
				working_width-ow));
		fprintf(fhp, "const double	PHASE_VARIANCE_RAD = %.4e; // (Radians^2)\n",
			phase_variance(nstages, phase_bits));
		fprintf(fhp, "const double	GAIN = %.16f;\n",
			cordic_gain(nstages));
		{
			double	amplitude = (1ul<<(iw-1))-1.,
				signal_energy, noise_energy;
			amplitude *= (1ul<<((working_width-iw)));
			amplitude *= cordic_gain(nstages);
			amplitude *= pow(2.0,-(working_width-ow));
			signal_energy = amplitude * amplitude;

			noise_energy = transform_quantization_variance(nstages,
				working_width-iw, working_width-ow);

			noise_energy += signal_energy * phase_variance(nstages, phase_bits)
				* pow(2,cordic_gain(nstages));

			fprintf(fhp, "const double\tBEST_POSSIBLE_CNR = %.2f;\n",
				10.0 * log(signal_energy / noise_energy)
					/log(10.0));
		}
		fprintf(fhp, "const bool\tHAS_RESET = %s;\n", with_reset?"true":"false");
		fprintf(fhp, "const bool\tHAS_AUX   = %s;\n", with_aux?"true":"false");
		if (with_reset)
			fprintf(fhp, "#define\tHAS_RESET_WIRE\n");
		if (with_aux)
			fprintf(fhp, "#define\tHAS_AUX_WIRES\n");
		fprintf(fhp, "#endif\t// %s\n", str);
		delete[] str;
	}
}

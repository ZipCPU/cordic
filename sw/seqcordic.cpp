////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	seqcordic.cpp
// {{{
// Project:	A series of CORDIC related projects
//
// Purpose:	Generates a CORDIC module that is sequential, rather than
// 		pipelined.  The resulting module may take several clocks to
// 	complete, but should also take much less logic along the way.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2018-2020, Gisselquist Technology, LLC
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
//
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
// }}}
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <string>
#include <ctype.h>
#include <assert.h>

#include "legal.h"
#include "cordiclib.h"
#include "basiccordic.h"
#include "seqcordic.h"

void	seqcordic(FILE *fp, FILE *fhp, const char *cmdline, const char *fname,
		int nstages, int iw, int ow, int nxtra,
		int phase_bits,
		bool with_reset, bool with_aux, bool async_reset) {
// {{{
	int	working_width = iw;
	const	char *name;
	const	char PURPOSE[] =
	"This file executes a vector rotation on the values\n"
	"//\t\t(i_xval, i_yval).  This vector is rotated left by\n"
	"//\ti_phase.  i_phase is given by the angle, in radians, multiplied by\n"
	"//\t2^32/(2pi).  In that fashion, a two pi value is zero just as a zero\n"
	"//\tangle is zero.\n//\n"
	"//\tThis particular version of the CORDIC processes one value at a\n"
	"//\ttime in a sequential, vs pipelined or parallel, fashion.",
		HPURPOSE[] =
	"This .h file notes the default parameter values from\n"
	"//\t\twithin the generated seqcordic file.  It is used to communicate\n"
	"//\tinformation about the design to the bench testing code.";
	legal(fp, fname, PROJECT, PURPOSE, cmdline);
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
	// {{{
	fprintf(fp, "`default_nettype\tnone\n");
	fprintf(fp,
		"module	%s #(\n"
		"\t\t// {{{\n"
		"\t\t// These parameters are fixed by the core generator.  They\n"
		"\t\t// have been used in the definitions of internal constants,\n"
		"\t\t// so they can\'t really be changed here.\n"
		"\t\tlocalparam\tIW=%2d,\t// The number of bits in our inputs\n"
		"\t\t\t\tOW=%2d,\t// The number of output bits to produce\n"
		"\t\t\t\tNSTAGES=%2d,\n"
		"\t\t\t\tXTRA=%2d,// Extra bits for internal precision\n"
		"\t\t\t\tWW=%2d,\t// Our working bit-width\n"
		"\t\t\t\tPW=%2d\t// Bits in our phase variables\n"
		"\t\t// }}}\n",
		name,
		iw, ow, nstages, nxtra, working_width, phase_bits);
	fprintf(fp,
		"\t) (\n"
		"\t\t// {{{\n"
		"\t\tinput\twire\t\t\t\ti_clk, %s%si_stb,%s\n"
		"\t\tinput\twire\tsigned\t[(IW-1):0]\ti_xval, i_yval,\n"
		"\t\tinput\twire\t\t[(PW-1):0]\ti_phase,\n"
		"\t\toutput\twire\t\t\t\to_busy,\n"
		"\t\toutput\treg\t\t\t\to_done,\n"
		"\t\toutput\treg\tsigned\t[(OW-1):0]\to_xval, o_yval%s\n"
		"\t\t// }}}\n"
		"\t);\n",
		//
		resetw.c_str(), (with_reset)?", ":"",
		(with_aux)?"\n\t\tinput\twire\t\t\t\ti_aux,":"",
		(with_aux)?",\n\t\toutput\treg\t\t\t\to_aux" : ""
	);

	fprintf(fp,
		"\t// First step: expand our input to our working width.\n"
		"\t// {{{\n"
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
	fprintf(fp, "\t// }}}\n");

	fprintf(fp,
		"\t// Declare variables for all of the separate stages\n"
		"\t// {{{\n");

	fprintf(fp,
		"\treg	signed	[(WW-1):0]	xv, prex, yv, prey;\n"
		"\treg		[(PW-1):0]	ph, preph;\n");
	fprintf(fp, "\treg\t\t\t\tidle, pre_valid;\n");
	fprintf(fp, "\treg\t\t[%d:0]\t\tstate;\n\n",
		nextlg((unsigned)nstages)-1);

	if (with_aux)
		fprintf(fp, "\treg\t\t\t\taux;\n");
	fprintf(fp,
		"\t// }}}\n\n");

	if (with_aux) {
		fprintf(fp,
"\t//\n"
"\t// Handle the auxilliary logic.\n"
"\t// {{{\n"
"\t// The auxilliary bit is designed so that you can place a valid bit into\n"
"\t// the CORDIC function, and see when it comes out.  While the bit is\n"
"\t// allowed to be anything, the requirement of this bit is that it *must*\n"
"\t// be aligned with the output when done.  That is, if i_xval and i_yval\n"
"\t// are input together with i_aux, then when o_xval and o_yval are set\n"
"\t// to this value, o_aux *must* contain the value that was in i_aux.\n"
"\t//\n"
"\n");

		fprintf(fp,
			"\tinitial\taux = 0;\n");
		fprintf(fp, "%s", always_reset.c_str());

		if (with_reset)
			fprintf(fp,
				"\t\taux <= 0;\n\telse ");
		fprintf(fp, "if ((i_stb)&&(!o_busy))\n"
			"\t\taux <= i_aux;\n\t// }}}\n"
			"\n");
	}

	fprintf(fp,
		"\t// First step, get rid of all but the last 45 degrees\n"
		"\t// {{{\n"
		"\t// The resulting phase needs to be between -45 and 45\n"
		"\t// degrees but in units of normalized phase\n\t//\n"
		"\t// We\'ll do this by walking through all possible quick phase\n"
		"\t// shifts necessary to constrain the input to within +/- 45\n"
		"\t// degrees.\n");
	fprintf(fp, "\talways @(posedge i_clk)\n");

	fprintf(fp,
		"\tcase(i_phase[(PW-1):(PW-3)])\n");

	fprintf(fp,
		"\t3'b000: begin	// 0 .. 45, No change\n"
		"\t\t// {{{\n"
		"\t\tprex  <=  e_xval;\n"
		"\t\tprey  <=  e_yval;\n"
		"\t\tpreph <= i_phase;\n"
		"\t\tend\n"
		"\t\t// }}}\n");

	fprintf(fp,
		"\t3'b001: begin	// 45 .. 90\n"
		"\t\t// {{{\n"
		"\t\tprex  <= -e_yval;\n"
		"\t\tprey  <=  e_xval;\n"
		"\t\tpreph <= i_phase - %d\'h%lx;\n"
		"\t\tend\n"
		"\t\t// }}}\n",
			phase_bits, (1ul << (phase_bits-2)));

	fprintf(fp,
		"\t3'b010: begin	// 90 .. 135\n"
		"\t\t// {{{\n"
		"\t\tprex  <= -e_yval;\n"
		"\t\tprey  <=  e_xval;\n"
		"\t\tpreph <= i_phase - %d\'h%lx;\n"
		"\t\tend\n"
		"\t\t// }}}\n",
			phase_bits, (1ul << (phase_bits-2)));

	fprintf(fp,
		"\t3'b011: begin	// 135 .. 180\n"
		"\t\t// {{{\n"
		"\t\tprex  <= -e_xval;\n"
		"\t\tprey  <= -e_yval;\n"
		"\t\tpreph <= i_phase - %d\'h%lx;\n"
		"\t\tend\n"
		"\t\t// }}}\n",
			phase_bits, (2ul << (phase_bits-2)));

	fprintf(fp,
		"\t3'b100: begin	// 180 .. 225\n"
		"\t\t// {{{\n"
		"\t\tprex  <= -e_xval;\n"
		"\t\tprey  <= -e_yval;\n"
		"\t\tpreph <= i_phase - %d\'h%lx;\n"
		"\t\tend\n"
		"\t\t// }}}\n",
			phase_bits, (2ul << (phase_bits-2)));

	fprintf(fp,
		"\t3'b101: begin	// 225 .. 270\n"
		"\t\t// {{{\n"
		"\t\tprex  <=  e_yval;\n"
		"\t\tprey  <= -e_xval;\n"
		"\t\tpreph <= i_phase - %d\'h%lx;\n"
		"\t\tend\n"
		"\t\t// }}}\n",
		phase_bits, (3ul << (phase_bits-2)));

	fprintf(fp,
		"\t3'b110: begin	// 270 .. 315\n"
		"\t\t// {{{\n"
		"\t\tprex  <=  e_yval;\n"
		"\t\tprey  <= -e_xval;\n"
		"\t\tpreph <= i_phase - %d\'h%lx;\n"
		"\t\tend\n"
		"\t\t// }}}\n",
		phase_bits, (3ul << (phase_bits-2)));

	fprintf(fp,
		"\t3'b111: begin	// 315 .. 360, No change\n"
		"\t\t// {{{\n"
		"\t\tprex  <=  e_xval;\n"
		"\t\tprey  <=  e_yval;\n"
		"\t\tpreph <= i_phase;\n"
		"\t\tend\n"
		"\t\t// }}}\n");

	fprintf(fp,
		"\tendcase\n"
		"\t// }}}\n\n");

	cordic_angles(fp, nstages, phase_bits, true);

	fprintf(fp, "\n\t// idle\n\t// {{{\n"
		"\tinitial\tidle = 1\'b1;\n");
	fprintf(fp, "%s", always_reset.c_str());
	if (with_reset)
		fprintf(fp, "\t\tidle <= 1\'b1;\n");
	fprintf(fp, "\telse if (i_stb)\n"
			"\t\tidle <= 1\'b0;\n"
			"\telse if (state == %d)\n"
			"\t\tidle <= 1\'b1;\n",
			nstages-1);
	fprintf(fp, "\t// }}}\n\n");

	fprintf(fp, "\t// pre_valid\n\t// {{{\n");
	fprintf(fp, "\tinitial\tpre_valid = 1\'b0;\n");
	fprintf(fp, "%s", always_reset.c_str());
	if (with_reset)
		fprintf(fp, "\t\tpre_valid <= 1\'b0;\n");
	fprintf(fp, "\telse\n\t\tpre_valid <= (i_stb)&&(idle);\n\t// }}}\n\n");

	fprintf(fp, "\t// cangle - CORDIC angle table lookup\n"
		"\t// {{{\n"
		"\talways @(posedge i_clk)\n"
			"\t\tcangle <= cordic_angle[state];\n"
		"\t// }}}\n\n");

	fprintf(fp, "\t// state\n\t// {{{\n\tinitial\tstate = 0;\n");
	fprintf(fp, "%s", always_reset.c_str());
	if (with_reset)
		fprintf(fp,
				"\t\tstate <= 0;\n\telse ");
	else
		fprintf(fp, "\t");
	fprintf(fp, "if (idle)\n"
			"\t\tstate <= 0;\n"
			"\telse if (state == %d)\n"
			"\t\tstate <= 0;\n"
			"\telse\n"
			"\t\tstate <= state + 1;\n\t// }}}\n\n", nstages-1);

	fprintf(fp,
		"\t// CORDIC rotations\n"
		"\t// {{{\n"
		"\t// Here\'s where we are going to put the actual CORDIC\n"
		"\t// we\'ve been studying and discussing.  Everything up to\n"
		"\t// this point has simply been necessary preliminaries.\n");
	fprintf(fp, "\talways @(posedge i_clk)\n"
		"\tif (pre_valid)\n"
		"\tbegin\n"
			"\t\t// {{{\n"
			"\t\txv <= prex;\n"
			"\t\tyv <= prey;\n"
			"\t\tph <= preph;\n"
			"\t\t// }}}\n"
		"\tend else if (ph[PW-1])\n"
		"\tbegin\n"
			"\t\t// {{{\n"
			"\t\txv <= xv + (yv >>> state);\n"
			"\t\tyv <= yv - (xv >>> state);\n"
			"\t\tph <= ph + (cangle);\n"
			"\t\t// }}}\n"
		"\tend else begin\n"
			"\t\t// {{{\n"
			"\t\txv <= xv - (yv >>> state);\n"
			"\t\tyv <= yv + (xv >>> state);\n"
			"\t\tph <= ph - (cangle);\n"
			"\t\t// }}}\n"
		"\tend\n\t// }}}\n");

	if (working_width > ow+1) {
		fprintf(fp,
			"\n\t// Round our result towards even\n"
			"\t// {{{\n"
			"\twire\t[(WW-1):0]\tfinal_xv, final_yv;\n\n"
			"\tassign\tfinal_xv = xv + $signed({{(OW){1\'b0}},\n"
				"\t\t\t\txv[(WW-OW)],\n"
				"\t\t\t\t{(WW-OW-1){!xv[WW-OW]}} });\n"
			"\tassign\tfinal_yv = yv + $signed({{(OW){1\'b0}},\n"
				"\t\t\t\tyv[(WW-OW)],\n"
				"\t\t\t\t{(WW-OW-1){!yv[WW-OW]}} });\n"
			"\t// }}}\n");

		fprintf(fp, "\t// o_done\n\t// {{{\n"
			"\tinitial\to_done = 1\'b0;\n");
		fprintf(fp, "%s", always_reset.c_str());

		if (with_reset)
			fprintf(fp, "\t\to_done <= 1\'b0;\n"
				"\telse\n");
		fprintf(fp, "\t\to_done <= (state >= %d);\n\t// }}}\n\n",
			nstages-1);

		fprintf(fp, "\t// Output assignments: o_xval, o_yval%s\n"
			"\t// {{{\n", (with_aux) ? ", o_aux":"");
		if (with_aux)
			fprintf(fp, "\tinitial\to_aux = 0;\n");
		fprintf(fp, "\talways @(posedge i_clk)\n"
			"\tif (state >= %d)\n"
			"\tbegin\n"
			"\t\to_xval <= final_xv[WW-1:WW-OW];\n"
			"\t\to_yval <= final_yv[WW-1:WW-OW];\n", nstages-1);
		if (with_aux)
			fprintf(fp,
			"\t\to_aux <= aux;\n");
		fprintf(fp, "\tend\n"
			"\t// }}}\n\n");

	} else {

		fprintf(fp, "\n\t// Output assignments: o_xval, o_yval%s\n"
			"\t// {{{\n",
			(with_aux) ? ", o_aux":"");
		fprintf(fp, "%s", always_reset.c_str());
		if (with_reset) {
			fprintf(fp,
			"\tbegin\n"
			"\t\to_xval <= 0;\n"
			"\t\to_yval <= 0;\n");
			if (with_aux)
				fprintf(fp, "\t\to_aux  <= 0;\n");
			fprintf(fp,
			"\tend else ");
		}

		fprintf(fp,
			"if (i_ce)\n"
			"\tbegin\t// We accumulate a bit during our processing, so shift by one\n"
			"\t\to_xval <= xv[(WW-1):(WW-OW)];\n"
			"\t\to_yval <= yv[(WW-1):(WW-OW)];\n");
		if (with_aux)
			fprintf(fp, "\t\to_aux  <= aux;\n");
		fprintf(fp, "\tend\n\t// }}}\n\n");
	}

	fprintf(fp, "\tassign\to_busy = !idle;\n\n");

	if (working_width > ow+1) {
		// {{{
		fprintf(fp, "\t// Make Verilator happy with pre_.val\n"
			"\t// {{{\n"
			"\t// verilator lint_off UNUSED\n"
			"\twire\tunused_val;\n"
			"\tassign\tunused_val = &{ 1\'b0, "
			" final_xv[WW-OW-1:0], final_yv[WW-OW-1:0] };\n"
			"\t// verilator lint_on UNUSED\n"
			"\t// }}}\n");
		// }}}
	}

	fprintf(fp, "endmodule\n");


	if (NULL != fhp) {
		// {{{
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
		fprintf(fhp, "#define\tCLOCKS_PER_OUTPUT\t%d\n\n", nstages+1);

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
		// }}}
	}
// }}}
}

////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	../rtl/cordic.v
//
// Project:	A series of CORDIC related projects
//
// Purpose:	This file executes a vector rotation on the values
//		(i_xval, i_yval).  This vector is rotated left by
//	i_phase.  i_phase is given by the angle, in radians, multiplied by
//	2^32/(2pi).  In that fashion, a two pi value is zero just as a zero
//	angle is zero.
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
`default_nettype	none
//
module	cordic(i_clk, i_reset, i_ce, i_xval, i_yval, i_phase, i_aux,
		o_xval, o_yval, o_aux);
	localparam	IW=12,	// The number of bits in our inputs
			OW=12,	// The number of output bits to produce
			NSTAGES=15,
			XTRA= 3,// Extra bits for internal precision
			WW=15,	// Our working bit-width
			PW=19;	// Bits in our phase variables
	input	wire				i_clk, i_reset, i_ce;
	input	wire	signed	[(IW-1):0]		i_xval, i_yval;
	input	wire		[(PW-1):0]			i_phase;
	output	reg	signed	[(OW-1):0]	o_xval, o_yval;
	input	wire				i_aux;
	output	reg				o_aux;
	// First step: expand our input to our working width.
	// This is going to involve extending our input by one
	// (or more) bits in addition to adding any xtra bits on
	// bits on the right.  The one bit extra on the left is to
	// allow for any accumulation due to the cordic gain
	// within the algorithm.
	// 
	wire	signed [(WW-1):0]	e_xval, e_yval;
	assign	e_xval = { {i_xval[(IW-1)]}, i_xval, {(WW-IW-1){1'b0}} };
	assign	e_yval = { {i_yval[(IW-1)]}, i_yval, {(WW-IW-1){1'b0}} };

	// Declare variables for all of the separate stages
	reg	signed	[(WW-1):0]	xv	[0:(NSTAGES)];
	reg	signed	[(WW-1):0]	yv	[0:(NSTAGES)];
	reg		[(PW-1):0]	ph	[0:(NSTAGES)];

	//
	// Handle the auxilliary logic.
	//
	// The auxilliary bit is designed so that you can place a valid bit into
	// the CORDIC function, and see when it comes out.  While the bit is
	// allowed to be anything, the requirement of this bit is that it *must*
	// be aligned with the output when done.  That is, if i_xval and i_yval
	// are input together with i_aux, then when o_xval and o_yval are set
	// to this value, o_aux *must* contain the value that was in i_aux.
	//
	reg		[(NSTAGES):0]	ax;

	always @(posedge i_clk)
	if (i_reset)
		ax <= {(NSTAGES+1){1'b0}};
	else if (i_ce)
		ax <= { ax[(NSTAGES-1):0], i_aux };

	// First stage, get rid of all but 45 degrees
	//	The resulting phase needs to be between -45 and 45
	//		degrees but in units of normalized phase
	always @(posedge i_clk)
	if (i_reset)
	begin
		xv[0] <= 0;
		yv[0] <= 0;
		ph[0] <= 0;
	end else if (i_ce)
	begin
		// Walk through all possible quick phase shifts necessary
		// to constrain the input to within +/- 45 degrees.
		case(i_phase[(PW-1):(PW-3)])
		3'b000: begin	// 0 .. 45, No change
			xv[0] <= e_xval;
			yv[0] <= e_yval;
			ph[0] <= i_phase;
			end
		3'b001: begin	// 45 .. 90
			xv[0] <= -e_yval;
			yv[0] <= e_xval;
			ph[0] <= i_phase - 19'h20000;
			end
		3'b010: begin	// 90 .. 135
			xv[0] <= -e_yval;
			yv[0] <= e_xval;
			ph[0] <= i_phase - 19'h20000;
			end
		3'b011: begin	// 135 .. 180
			xv[0] <= -e_xval;
			yv[0] <= -e_yval;
			ph[0] <= i_phase - 19'h40000;
			end
		3'b100: begin	// 180 .. 225
			xv[0] <= -e_xval;
			yv[0] <= -e_yval;
			ph[0] <= i_phase - 19'h40000;
			end
		3'b101: begin	// 225 .. 270
			xv[0] <= e_yval;
			yv[0] <= -e_xval;
			ph[0] <= i_phase - 19'h60000;
			end
		3'b110: begin	// 270 .. 315
			xv[0] <= e_yval;
			yv[0] <= -e_xval;
			ph[0] <= i_phase - 19'h60000;
			end
		3'b111: begin	// 315 .. 360, No change
			xv[0] <= e_xval;
			yv[0] <= e_yval;
			ph[0] <= i_phase;
			end
		endcase
	end

	//
	// In many ways, the key to this whole algorithm lies in the angles
	// necessary to do this.  These angles are also our basic reason for
	// building this CORDIC in C++: Verilog just can't parameterize this
	// much.  Further, these angle's risk becoming unsupportable magic
	// numbers, hence we define these and set them in C++, based upon
	// the needs of our problem, specifically the number of stages and
	// the number of bits required in our phase accumulator
	//
	wire	[18:0]	cordic_angle [0:(NSTAGES-1)];

	assign	cordic_angle[ 0] = 19'h0_9720; //  26.565051 deg
	assign	cordic_angle[ 1] = 19'h0_4fd9; //  14.036243 deg
	assign	cordic_angle[ 2] = 19'h0_2888; //   7.125016 deg
	assign	cordic_angle[ 3] = 19'h0_1458; //   3.576334 deg
	assign	cordic_angle[ 4] = 19'h0_0a2e; //   1.789911 deg
	assign	cordic_angle[ 5] = 19'h0_0517; //   0.895174 deg
	assign	cordic_angle[ 6] = 19'h0_028b; //   0.447614 deg
	assign	cordic_angle[ 7] = 19'h0_0145; //   0.223811 deg
	assign	cordic_angle[ 8] = 19'h0_00a2; //   0.111906 deg
	assign	cordic_angle[ 9] = 19'h0_0051; //   0.055953 deg
	assign	cordic_angle[10] = 19'h0_0028; //   0.027976 deg
	assign	cordic_angle[11] = 19'h0_0014; //   0.013988 deg
	assign	cordic_angle[12] = 19'h0_000a; //   0.006994 deg
	assign	cordic_angle[13] = 19'h0_0005; //   0.003497 deg
	assign	cordic_angle[14] = 19'h0_0002; //   0.001749 deg
	// Std-Dev    : 0.00 (Units)
	// Phase Quantization: 0.000030 (Radians)
	// Gain is 1.164435
	// You can annihilate this gain by multiplying by 32'hdbd95b17
	// and right shifting by 32 bits.

	genvar	i;
	generate for(i=0; i<NSTAGES; i=i+1) begin : CORDICops
		always @(posedge i_clk)
		// Here's where we are going to put the actual CORDIC
		// we've been studying and discussing.  Everything up to
		// this point has simply been necessary preliminaries.
		if (i_reset)
		begin
			xv[i+1] <= 0;
			yv[i+1] <= 0;
			ph[i+1] <= 0;
		end else if (i_ce)
		begin
			if ((cordic_angle[i] == 0)||(i >= WW))
			begin // Do nothing but move our outputs
			// forward one stage, since we have more
			// stages than valid data
				xv[i+1] <= xv[i];
				yv[i+1] <= yv[i];
				ph[i+1] <= ph[i];
			end else if (ph[i][(PW-1)]) // Negative phase
			begin
				// If the phase is negative, rotate by the
				// CORDIC angle in a clockwise direction.
				xv[i+1] <= xv[i] + (yv[i]>>>(i+1));
				yv[i+1] <= yv[i] - (xv[i]>>>(i+1));
				ph[i+1] <= ph[i] + cordic_angle[i];
			end else begin
				// On the other hand, if the phase is
				// positive ... rotate in the
				// counter-clockwise direction
				xv[i+1] <= xv[i] - (yv[i]>>>(i+1));
				yv[i+1] <= yv[i] + (xv[i]>>>(i+1));
				ph[i+1] <= ph[i] - cordic_angle[i];
			end
		end
	end endgenerate

	// Round our result towards even
	wire	[(WW-1):0]	pre_xval, pre_yval;

	assign	pre_xval = xv[NSTAGES] + $signed({{(OW){1'b0}},
				xv[NSTAGES][(WW-OW)],
				{(WW-OW-1){!xv[NSTAGES][WW-OW]}}});
	assign	pre_yval = yv[NSTAGES] + $signed({{(OW){1'b0}},
				yv[NSTAGES][(WW-OW)],
				{(WW-OW-1){!yv[NSTAGES][WW-OW]}}});

	always @(posedge i_clk)
	if (i_reset)
	begin
		o_xval <= 0;
		o_yval <= 0;
	end else if (i_ce)
	begin
		o_xval <= pre_xval[(WW-1):(WW-OW)];
		o_yval <= pre_yval[(WW-1):(WW-OW)];
		o_aux <= ax[NSTAGES];
	end

	// Make Verilator happy with pre_.val
	// verilator lint_off UNUSED
	wire	[(2*(WW-OW)-1):0] unused_val;
	assign	unused_val = {
		pre_xval[(WW-OW-1):0],
		pre_yval[(WW-OW-1):0]
		};
	// verilator lint_on UNUSED
endmodule

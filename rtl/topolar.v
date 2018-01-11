////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	../rtl/topolar.v
//
// Project:	A series of CORDIC related projects
//
// Purpose:	This is a rectangular to polar conversion routine based upon an
//		internal CORDIC implementation.  Basically, the input is
//	provided in i_xval and i_yval.  The internal CORDIC rotator will rotate
//	(i_xval, i_yval) until i_yval is approximately zero.  The resulting
//	xvalue and phase will be placed into o_xval and o_phase respectively.
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
module	topolar(i_clk, i_reset, i_ce, i_xval, i_yval, i_aux,
		o_mag, o_phase, o_aux);
	localparam	IW=12,	// The number of bits in our inputs
			OW=12,// The number of output bits to produce
			NSTAGES=16,
			XTRA= 3,// Extra bits for internal precision
			WW=18,	// Our working bit-width
			PW=19;	// Bits in our phase variables
	input					i_clk, i_reset, i_ce;
	input	wire	signed	[(IW-1):0]	i_xval, i_yval;
	output	reg	signed	[(OW-1):0]	o_mag;
	output	reg		[(PW-1):0]	o_phase;
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
	assign	e_xval = { {(2){i_xval[(IW-1)]}}, i_xval, {(WW-IW-2){1'b0}} };
	assign	e_yval = { {(2){i_yval[(IW-1)]}}, i_yval, {(WW-IW-2){1'b0}} };

	// Declare variables for all of the separate stages
	reg	signed	[(WW-1):0]	xv	[0:NSTAGES];
	reg	signed	[(WW-1):0]	yv	[0:NSTAGES];
	reg		[(PW-1):0]	ph	[0:NSTAGES];

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

	// First stage, map to within +/- 45 degrees
	always @(posedge i_clk)
	if (i_reset)
	begin
		xv[0] <= 0;
		yv[0] <= 0;
		ph[0] <= 0;
	end else if (i_ce)
		case({i_xval[IW-1], i_yval[IW-1]})
		2'b01: begin // Rotate by -315 degrees
			xv[0] <=  e_xval - e_yval;
			yv[0] <=  e_xval + e_yval;
			ph[0] <= 19'h70000;
			end
		2'b10: begin // Rotate by -135 degrees
			xv[0] <= -e_xval + e_yval;
			yv[0] <= -e_xval - e_yval;
			ph[0] <= 19'h30000;
			end
		2'b11: begin // Rotate by -225 degrees
			xv[0] <= -e_xval - e_yval;
			yv[0] <=  e_xval - e_yval;
			ph[0] <= 19'h50000;
			end
		// 2'b00:
		default: begin // Rotate by -45 degrees
			xv[0] <=  e_xval + e_yval;
			yv[0] <= -e_xval + e_yval;
			ph[0] <= 19'h10000;
			end
		endcase
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
	assign	cordic_angle[15] = 19'h0_0001; //   0.000874 deg
	// Std-Dev    : 0.00 (Units)
	// Phase Quantization: 0.000030 (Radians)
	// Gain is 1.164435
	// You can annihilate this gain by multiplying by 32'hdbd95b16
	// and right shifting by 32 bits.

	genvar	i;
	generate for(i=0; i<NSTAGES; i=i+1) begin : TOPOLARloop
		always @(posedge i_clk)
		// Here's where we are going to put the actual CORDIC
		// rectangular to polar loop.  Everything up to this
		// point has simply been necessary preliminaries.
		if (i_reset)
		begin
			xv[i+1] <= 0;
			yv[i+1] <= 0;
			ph[i+1] <= 0;
		end else if (i_ce)
		begin
			if ((cordic_angle[i] == 0)||(i >= WW))
			begin // Do nothing but move our vector
			// forward one stage, since we have more
			// stages than valid data
				xv[i+1] <= xv[i];
				yv[i+1] <= yv[i];
				ph[i+1] <= ph[i];
			end else if (yv[i][(WW-1)]) // Below the axis
			begin
				// If the vector is below the x-axis, rotate by
				// the CORDIC angle in a positive direction.
				xv[i+1] <= xv[i] - (yv[i]>>>(i+1));
				yv[i+1] <= yv[i] + (xv[i]>>>(i+1));
				ph[i+1] <= ph[i] - cordic_angle[i];
			end else begin
				// On the other hand, if the vector is above the
				// x-axis, then rotate in the other direction
				xv[i+1] <= xv[i] + (yv[i]>>>(i+1));
				yv[i+1] <= yv[i] - (xv[i]>>>(i+1));
				ph[i+1] <= ph[i] + cordic_angle[i];
			end
		end
	end endgenerate

	// Round our magnitude towards even
	wire	[(WW-1):0]	pre_mag;

	assign	pre_mag = xv[NSTAGES] + $signed({{(OW){1'b0}},
				xv[NSTAGES][(WW-OW)],
				{(WW-OW-1){!xv[NSTAGES][WW-OW]}}});

	always @(posedge i_clk)
	if (i_reset)
	begin
		o_mag   <= 0;
		o_phase <= 0;
		o_aux <= 0;
	end else if (i_ce)
	begin
		o_mag   <= pre_mag[(WW-1):(WW-OW)];
		o_phase <= ph[NSTAGES];
		o_aux <= ax[NSTAGES];
	end

	// Make Verilator happy with pre_.val
	// verilator lint_off UNUSED
	wire	[(WW-OW):0] unused_val;
	assign	unused_val = { pre_mag[WW-1], pre_mag[(WW-OW-1):0] };
	// verilator lint_on UNUSED
endmodule

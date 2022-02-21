////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	../rtl/topolar.v
// {{{
// Project:	A series of CORDIC related projects
//
// Purpose:	This is a rectangular to polar conversion routine based upon an
//		internal CORDIC implementation.  Basically, the input is
//	provided in i_xval and i_yval.  The internal CORDIC rotator will rotate
//	(i_xval, i_yval) until i_yval is approximately zero.  The resulting
//	xvalue and phase will be placed into o_xval and o_phase respectively.
//
// This core was generated via a core generator using the following command
// line:
//
//  % ./gencordic -vca -f ../rtl/topolar.v -i 13 -o 13 -t r2p -x 2
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2017-2022, Gisselquist Technology, LLC
// {{{
// This file is part of the CORDIC related project set.
//
// The CORDIC related project set is free software (firmware): you can
// redistribute it and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// The CORDIC related project set is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
// General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  (It's in the $(ROOT)/doc directory.  Run make
// with no target there if the PDF file isn't present.)  If not, see
// License:	LGPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/lgpl.html
//
////////////////////////////////////////////////////////////////////////////////
//
// }}}
`default_nettype	none
//
module	topolar #(
		// {{{
		localparam	IW=13,	// The number of bits in our inputs
			OW=13,// The number of output bits to produce
			NSTAGES=18,
			// XTRA= 4,// Extra bits for internal precision
			WW=21,	// Our working bit-width
			PW=21	// Bits in our phase variables
		// }}}
	) (
		// {{{
	input	wire				i_clk, i_reset, i_ce,
	input	wire	signed	[(IW-1):0]	i_xval, i_yval,
	output	reg	signed	[(OW-1):0]	o_mag,
	output	reg		[(PW-1):0]	o_phase,
	input	wire				i_aux,
	output	reg				o_aux
		// }}}
	);
	// Declare variables for all of the separate stages
	// {{{
	wire	signed [(WW-1):0]	e_xval, e_yval;
	reg	signed	[(WW-1):0]	xv	[0:NSTAGES];
	reg	signed	[(WW-1):0]	yv	[0:NSTAGES];
	reg		[(PW-1):0]	ph	[0:NSTAGES];
	// }}}
	// Sign extension
	// {{{
	// First step: expand our input to our working width.
	// This is going to involve extending our input by one
	// (or more) bits in addition to adding any xtra bits on
	// bits on the right.  The one bit extra on the left is to
	// allow for any accumulation due to the cordic gain
	// within the algorithm.
	// 
	assign	e_xval = { {(2){i_xval[(IW-1)]}}, i_xval, {(WW-IW-2){1'b0}} };
	assign	e_yval = { {(2){i_yval[(IW-1)]}}, i_yval, {(WW-IW-2){1'b0}} };

	// }}}
	//
	// Handle the auxilliary logic.
	// {{{
	// The auxilliary bit is designed so that you can place a valid bit into
	// the CORDIC function, and see when it comes out.  While the bit is
	// allowed to be anything, the requirement of this bit is that it *must*
	// be aligned with the output when done.  That is, if i_xval and i_yval
	// are input together with i_aux, then when o_xval and o_yval are set
	// to this value, o_aux *must* contain the value that was in i_aux.
	//
	reg		[(NSTAGES):0]	ax;

	initial	ax = 0;
	always @(posedge i_clk)
	if (i_reset)
		ax <= 0;
	else if (i_ce)
		ax <= { ax[(NSTAGES-1):0], i_aux };

	// }}}
	// Pre-CORDIC rotation
	// {{{
	initial begin
		xv[0] = 0;
		yv[0] = 0;
		ph[0] = 0;
	end
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
		// {{{
		xv[0] <=  e_xval - e_yval;
		yv[0] <=  e_xval + e_yval;
		ph[0] <= 21'h1c0000;
		end
		// }}}
	2'b10: begin // Rotate by -135 degrees
		// {{{
		xv[0] <= -e_xval + e_yval;
		yv[0] <= -e_xval - e_yval;
		ph[0] <= 21'hc0000;
		end
		// }}}
	2'b11: begin // Rotate by -225 degrees
		// {{{
		xv[0] <= -e_xval - e_yval;
		yv[0] <=  e_xval - e_yval;
		ph[0] <= 21'h140000;
		end
		// }}}
	// 2'b00:
		// {{{
	default: begin // Rotate by -45 degrees
		xv[0] <=  e_xval + e_yval;
		yv[0] <= -e_xval + e_yval;
		ph[0] <= 21'h40000;
		end
		// }}}
	endcase
	// }}}
	// Cordic angle table
	// {{{
	// In many ways, the key to this whole algorithm lies in the angles
	// necessary to do this.  These angles are also our basic reason for
	// building this CORDIC in C++: Verilog just can't parameterize this
	// much.  Further, these angle's risk becoming unsupportable magic
	// numbers, hence we define these and set them in C++, based upon
	// the needs of our problem, specifically the number of stages and
	// the number of bits required in our phase accumulator
	//
	wire	[20:0]	cordic_angle [0:(NSTAGES-1)];

	assign	cordic_angle[ 0] = 21'h02_5c80; //  26.565051 deg
	assign	cordic_angle[ 1] = 21'h01_3f67; //  14.036243 deg
	assign	cordic_angle[ 2] = 21'h00_a222; //   7.125016 deg
	assign	cordic_angle[ 3] = 21'h00_5161; //   3.576334 deg
	assign	cordic_angle[ 4] = 21'h00_28ba; //   1.789911 deg
	assign	cordic_angle[ 5] = 21'h00_145e; //   0.895174 deg
	assign	cordic_angle[ 6] = 21'h00_0a2f; //   0.447614 deg
	assign	cordic_angle[ 7] = 21'h00_0517; //   0.223811 deg
	assign	cordic_angle[ 8] = 21'h00_028b; //   0.111906 deg
	assign	cordic_angle[ 9] = 21'h00_0145; //   0.055953 deg
	assign	cordic_angle[10] = 21'h00_00a2; //   0.027976 deg
	assign	cordic_angle[11] = 21'h00_0051; //   0.013988 deg
	assign	cordic_angle[12] = 21'h00_0028; //   0.006994 deg
	assign	cordic_angle[13] = 21'h00_0014; //   0.003497 deg
	assign	cordic_angle[14] = 21'h00_000a; //   0.001749 deg
	assign	cordic_angle[15] = 21'h00_0005; //   0.000874 deg
	assign	cordic_angle[16] = 21'h00_0002; //   0.000437 deg
	assign	cordic_angle[17] = 21'h00_0001; //   0.000219 deg
	// {{{
	// Std-Dev    : 0.00 (Units)
	// Phase Quantization: 0.000008 (Radians)
	// Gain is 1.164435
	// You can annihilate this gain by multiplying by 32'hdbd95b16
	// and right shifting by 32 bits.
	// }}}
	// }}}

	// Actual CORDIC rotations
	// {{{
	genvar	i;
	generate for(i=0; i<NSTAGES; i=i+1) begin : TOPOLARloop
		initial begin
			xv[i+1] = 0;
			yv[i+1] = 0;
			ph[i+1] = 0;
		end

		always @(posedge i_clk)
		// Here's where we are going to put the actual CORDIC
		// rectangular to polar loop.  Everything up to this
		// point has simply been necessary preliminaries.
		if (i_reset)
		begin
			// {{{
			xv[i+1] <= 0;
			yv[i+1] <= 0;
			ph[i+1] <= 0;
			// }}}
		end else if (i_ce)
		begin
			// {{{
			if ((cordic_angle[i] == 0)||(i >= WW))
			begin // Do nothing but move our vector
			// forward one stage, since we have more
			// stages than valid data
				// {{{
				xv[i+1] <= xv[i];
				yv[i+1] <= yv[i];
				ph[i+1] <= ph[i];
				// }}}
			end else if (yv[i][(WW-1)]) // Below the axis
			begin
				// {{{
				// If the vector is below the x-axis, rotate by
				// the CORDIC angle in a positive direction.
				xv[i+1] <= xv[i] - (yv[i]>>>(i+1));
				yv[i+1] <= yv[i] + (xv[i]>>>(i+1));
				ph[i+1] <= ph[i] - cordic_angle[i];
				// }}}
			end else begin
				// {{{
				// On the other hand, if the vector is above the
				// x-axis, then rotate in the other direction
				xv[i+1] <= xv[i] + (yv[i]>>>(i+1));
				yv[i+1] <= yv[i] - (xv[i]>>>(i+1));
				ph[i+1] <= ph[i] + cordic_angle[i];
				// }}}
			end
			// }}}
		end
	end endgenerate
	// }}}

	// Round our magnitude towards even
	// {{{
	wire	[(WW-1):0]	pre_mag;

	assign	pre_mag = xv[NSTAGES] + $signed({ {(OW){1'b0}},
				xv[NSTAGES][(WW-OW)],
				{(WW-OW-1){!xv[NSTAGES][WW-OW]}} });

	initial	o_mag   = 0;
	initial	o_phase = 0;
	initial	o_aux   = 0;
	always @(posedge i_clk)
	if (i_reset)
	begin
		o_mag   <= 0;
		o_phase <= 0;
		o_aux   <= 0;
	end else if (i_ce)
	begin
		o_mag   <= pre_mag[(WW-1):(WW-OW)];
		o_phase <= ph[NSTAGES];
		o_aux <= ax[NSTAGES];
	end

	// Make Verilator happy with pre_.val
	// verilator lint_off UNUSED
	wire	unused_val;
	assign	unused_val = &{ 1'b0,  pre_mag[WW-1], pre_mag[(WW-OW-1):0] };
	// verilator lint_on UNUSED
	// }}}
endmodule

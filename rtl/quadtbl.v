////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	../rtl/quadtbl.v
// {{{
// Project:	A series of CORDIC related projects
//
// Purpose:	This is a sine-wave table lookup algorithm, coupled with a
//		quadratic interpolation of the result.  It's purpose is both
//	 to trade off logic, as well as to lower the phase noise associated
//	with any phase truncation.
//
// This core was generated via a core generator using the following command
// line:
//
//  % ./gencordic -vca -f ../rtl/quadtbl.v -p 18 -o 13 -t qtbl
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
module	quadtbl #(
		// {{{
		localparam	PW=18,	// Bits in our phase variable
				OW=13,  // The number of output bits to produce
				XTRA= 3 // Extra bits for internal precision
		// }}}
	) (
		// {{{
		input	wire				i_clk, i_reset, i_ce, i_aux,
		//
		input	wire	signed	[(PW-1):0]	i_phase,
		output	reg	signed	[(OW-1):0]	o_sin,
		output	reg				o_aux
		// }}}
	);

	// Declarations
	// {{{
	localparam	LGTBL=6,
			DXBITS  = (PW-LGTBL)+1,  // 13
			TBLENTRIES = (1<<LGTBL), // 64
			QBITS   = 9,
			LBITS   = 13,
			CBITS   = 16,
			WW      = (OW+XTRA), // Working width
			NSTAGES = 6; // Hard-coded to the algorithm

	//
	// Space for our coefficients, and their copies as we work through
	// our processing stages
	reg	signed	[(CBITS-1):0]	cv,
					cv_1, cv_2, cv_3;
	reg	signed	[(LBITS-1):0]	lv, lv_1;
	reg	signed	[(QBITS-1):0]	qv;
	reg	signed	[(DXBITS-1):0]	dx, dx_1, dx_2;

	//
	//
	reg	signed	[(QBITS+DXBITS-1):0]	qprod; // [21:0]
	reg		[(NSTAGES-1):0]		aux;
	reg	signed	[(LBITS-1):0]		lsum;
	reg	signed	[(LBITS+DXBITS-1):0]	lprod;
	wire		[(LBITS-1):0]		w_qprod;
	reg	signed	[(CBITS-1):0]		r_value; // 16 bits
	wire	signed	[(CBITS-1):0]		w_lprod;

	// Coefficient tables:
	//	Constant, Linear, and Quadratic
	reg	[(CBITS-1):0]	ctbl [0:(TBLENTRIES-1)]; //=(0...2^(OX)-1)/2^32
	reg	[(LBITS-1):0]	ltbl [0:(TBLENTRIES-1)]; // 13 x 64
	reg	[(QBITS-1):0]	qtbl [0:(TBLENTRIES-1)]; // 9 x 64

	reg	[(WW-1):0]	w_value;
	initial begin
		$readmemh("quadtbl_ctbl.hex", ctbl);
		$readmemh("quadtbl_ltbl.hex", ltbl);
		$readmemh("quadtbl_qtbl.hex", qtbl);
	end

	// }}}

	// aux, o_aux logic
	// {{{
	initial	aux = 0;
	always @(posedge i_clk)
	if (i_reset)
		aux <= 0;
	else if (i_ce)
			aux <= { aux[(NSTAGES-2):0], i_aux };
	assign	o_aux = aux[(NSTAGES-1)];
	// }}}

	////////////////////////////////////////////////////////////////////////
	//
	// Clock 1 - Table coefficient lookups
	// {{{
	//	1. Operate on the incoming bits--this is the only stage
	//	   that does so
	//	2. Read our coefficients from the table
	//	3. Store dx, the difference between the table value and the
	//		actually requested phase, for later processing
	//
	//
	initial	qv = 0;
	initial	lv = 0;
	initial	cv = 0;
	initial	dx = 0;
	always @(posedge i_clk)
	if (i_reset)
	begin
		qv <= 0;
		lv <= 0;
		cv <= 0;
		dx <= 0;
	end else if (i_ce)
	begin
		qv <= qtbl[i_phase[(PW-1):(DXBITS-1)]];
		lv <= ltbl[i_phase[(PW-1):(DXBITS-1)]];
		cv <= ctbl[i_phase[(PW-1):(DXBITS-1)]];
		dx <= { 1'b0, i_phase[(DXBITS-2):0] };	// * 2^(-PW)
	end

	//
	// Here's our formula:
	//
	//	 Out = (Q*DX+L)*DX+C
	//
	// A basic quadratic interpolant.  All of the smarts are found within
	// the Q, L, and C values.

	// }}}
	////////////////////////////////////////////////////////////////////////
	//
	// Clock 2 - Multiply by the quadratic coefficient
	// {{{
	//	1. Multiply to get the quadratic component of our design
	//		This is the first of two multiplies used by this
	//		algorithm
	//	2. Everything else is just copied to the next clock
	//
	//
	always @(posedge i_clk)
	if (i_ce)
		qprod <= qv * dx; // 22 bits

	initial	cv_1 = 0;
	initial	lv_1 = 0;
	initial	dx_1 = 0;
	always @(posedge i_clk)
	if (i_reset)
	begin
		cv_1 <= 0;
		lv_1 <= 0;
		dx_1 <= 0;
	end else if (i_ce) begin
		cv_1 <= cv;
		lv_1 <= lv;
		dx_1 <= dx;
	end

	// }}}
	////////////////////////////////////////////////////////////////////////
	//
	// Clock 3 - Add the result to the linear component
	// {{{
	//	1. Select the number of bits we want from the output
	//	2. Add our linear term to the result of the multiply
	//	3. Copy the remaining values for the next clock
	//
	//
	assign	w_qprod[(LBITS-1):(QBITS+1)] = { (3){qprod[(QBITS+DXBITS-1)]} };
	assign	w_qprod[QBITS:0] // 10
			= qprod[(QBITS+DXBITS-1):(DXBITS-1)]; // [21:12]
	initial	lsum = 0;
	always @(posedge i_clk)
	if (i_reset)
		lsum <= 0;
	else if (i_ce)
		lsum <= w_qprod + lv_1; // 14 bits

	initial	cv_2 = 0;
	initial	dx_2 = 0;
	always @(posedge i_clk)
	if (i_reset)
	begin
		cv_2 <= 0;
		dx_2 <= 0;
	end else if (i_ce) begin
		cv_2 <= cv_1;
		dx_2 <= dx_1;
	end

	// }}}
	////////////////////////////////////////////////////////////////////////
	//
	// Clock 4 - Last multiply, w/ the linear coefficient
	// {{{
	//	1. Our second and final multiply
	//	2. Copy the constant coefficient value to the next clock
	//
	//
	initial	lprod = 0;
	always @(posedge i_clk)
	if (i_ce)
		lprod <= lsum * dx_2; // 27 bits

	initial	cv_3 = 0;
	always @(posedge i_clk)
	if (i_reset)
		cv_3 <= 0;
	else if (i_ce)
		cv_3 <= cv_2;

	// }}}
	////////////////////////////////////////////////////////////////////////
	//
	// Clock 5 - Add in the constant
	// {{{
	//	1. Add the constant value to the result of the last
	//	   multiplication.  This will be the output of our algorithm
	//	2. There's nothing left to copy
	//
	//
	assign	w_lprod[(CBITS-1):(LBITS+1)] = { (2){lprod[(LBITS+DXBITS-1)]} };
	assign	w_lprod[(LBITS):0] = lprod[(LBITS+DXBITS-1):(DXBITS-1)]; // 13 bits
	initial	r_value = 0;
	always @(posedge i_clk)
	if (i_reset)
		r_value <= 0;
	else if (i_ce)
		r_value <= w_lprod + cv_3;

	// }}}
	////////////////////////////////////////////////////////////////////////
	//
	// Clock 6 - Round the output
	// {{{
	//	1. The last and final step is to round the output to the
	//	   nearest value.  This also involves dropping the extra bits
	//	   we've been carrying around since the last multiply.
	//
	//

	// Since we won't be using all of the bits in w_value, we'll just
	// mark them all as unused for Verilator's linting purposes
	//
	always @(*)
		if ((!r_value[WW-1])&&(&r_value[(WW-2):XTRA]))
			w_value = r_value;
		else if ((r_value[(WW-1):(WW-2)]==2'b11)&&(!|r_value[(WW-3):XTRA]))
			w_value = r_value;
		else
			w_value = r_value + { {(OW){1'b0}},
				r_value[(WW-OW)],
				{(WW-OW-1){!r_value[(WW-OW)]}} };
	// }}}
	//
	// Calculate the final result
	// {{{
	initial	o_sin = 0;
	always @(posedge i_clk)
	if (i_reset)
		o_sin <= 0;
	else if (i_ce)
		o_sin <= w_value[(WW-1):XTRA]; // [19:3]
	// }}}

	// Make verilator happy
	// {{{
	// verilator lint_off UNUSED
	wire	 unused;
	assign	unused = &{ 1'b0, w_value,
			lprod[(DXBITS-1):0],
			r_value[(XTRA-1):0],
			qprod[(DXBITS-1):0] };
	// verilator lint_on  UNUSED
	// }}}

endmodule

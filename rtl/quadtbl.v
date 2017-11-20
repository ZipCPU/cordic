////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	../rtl/quadtbl.v
//
// Project:	A series of CORDIC related projects
//
// Purpose:	This is a sine-wave table lookup algorithm, coupled with a quadratic
//		interpolation of the result.  It's purpose is both to trade
//	off logic, as well as to lower the phase noise associated with
//	any phase truncation.

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
module	quadtbl(i_clk, i_reset, i_ce,  i_aux,i_phase, o_sin, o_aux);
	localparam	PW=26,	// Bits in our phase variable
			OW=24,  // The number of output bits to produce
			XTRA= 3;// Extra bits for internal precision
	input	wire				i_clk, i_reset, i_ce, i_aux;
	//
	input	wire	signed	[(PW-1):0]	i_phase;
	output	reg	signed	[(OW-1):0]	o_sin;
	output	reg				o_aux;

	localparam	LGTBL=9,
			DXBITS= (PW-LGTBL)+1,    // 16
			TBLENTRIES = (1<<LGTBL), // 512
			QBITS = 14,
			LBITS = 21,
			CBITS = 27,
			WW    = (OW+XTRA); // Working width

	reg	signed	[(CBITS-1):0]	cv,
					cv_1, cv_2, cv_3;
	reg	signed	[(LBITS-1):0]	lv, lv_1;
	reg	signed	[(QBITS-1):0]	qv;
	reg	signed	[(DXBITS-1):0]	dx, dx_1, dx_2;

	reg	[(CBITS-1):0]	ctbl [0:(TBLENTRIES-1)]; //=(0...2^(OX)-1)/2^32
	// ltbl !=
	reg	[(LBITS-1):0]	ltbl [0:(TBLENTRIES-1)]; // 21 x 512
	reg	[(QBITS-1):0]	qtbl [0:(TBLENTRIES-1)]; // 14 x 512

	initial begin
		$readmemh("quadtbl_ctbl.hex", ctbl);
		$readmemh("quadtbl_ltbl.hex", ltbl);
		$readmemh("quadtbl_qtbl.hex", qtbl);
	end

	// aux bit
	localparam	NSTAGES=6;
	reg	[(NSTAGES-1):0]	aux;
	initial	aux = 0;
	always @(posedge i_clk)
		if (i_reset)
			aux <= 0;
		else if (i_ce)
			aux <= { aux[(NSTAGES-2):0], i_aux };
	assign	o_aux = aux[(NSTAGES-1)];

	// Clock zero
	always @(posedge i_clk)
		if (i_ce)
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

	// Clock 1
	reg	signed	[(QBITS+DXBITS-1):0]	qprod; // [29:0]
	always @(posedge i_clk)
		if (i_reset)
			qprod <= 0;
		else if (i_ce)
			qprod <= qv * dx; // 30 bits

	initial	cv_1 = 0;
	initial	lv_1 = 0;
	initial	dx_1 = 0;
	always @(posedge i_clk)
		if (i_ce) begin
			cv_1 <= cv;
			lv_1 <= lv;
			dx_1 <= dx;
		end

	// Clock 2
	reg	signed [(LBITS-1):0]	lsum;
	wire	[(LBITS-1):0]	w_qprod;
	assign	w_qprod[(LBITS-1):(QBITS+1)] = { (6){qprod[(QBITS+DXBITS-1)]} };
	assign	w_qprod[QBITS:0] // 15
			= qprod[(QBITS+DXBITS-1):(DXBITS-1)]; // [29:15]
	always @(posedge i_clk)
		if (i_ce)
			lsum <= w_qprod + lv_1; // 22 bits

	always @(posedge i_clk)
		if (i_ce) begin
			cv_2 <= cv_1;
			dx_2 <= dx_1;
		end

	// Clock 2
	reg	signed	[(LBITS+DXBITS-1):0]	lprod;
	always @(posedge i_clk)
		if (i_ce)
			lprod <= lsum * dx_2; // 38 bits

	initial	cv_3 = 0;
	always @(posedge i_clk)
		if (i_ce)
			cv_3 <= cv_2;

	// Clock 4
	reg	signed	[(CBITS-1):0]		r_value; // 27 bits
	wire	signed	[(CBITS-1):0]		w_lprod;
	assign	w_lprod[(CBITS-1):(LBITS+1)] = { (5){lprod[(LBITS+DXBITS-1)]} };
	assign	w_lprod[(LBITS):0] = lprod[(LBITS+DXBITS-1):(DXBITS-1)]; // 21 bits
	initial	r_value = 0;
	always @(posedge i_clk)
		if (i_ce)
			r_value <= w_lprod + cv_3;

	// Clock 5 - round the output
	// verilator lint_off UNUSED
	wire	[(WW-1):0]	w_value;
	always @(*)
		if ((!r_value[WW-1])&&(&r_value[(WW-2):XTRA]))
			w_value = r_value;
		else if ((r_value[(WW-1):(WW-2)]==2'b11)&&(!|r_value[(WW-3):XTRA]))
			w_value = r_value;
		else
			w_value = r_value + { {(OW){1'b0}},
				r_value[(WW-OW)],
				{(WW-OW-1){!r_value[(WW-OW)]}} };
	// verilator lint_on  UNUSED
	initial	o_sin = 0;
	always @(posedge i_clk)
		if (i_ce)
			o_sin <= w_value[(WW-1):XTRA]; // [30:3]

	// Make verilator happy
	// verilator lint_off UNUSED
	wire	[(2*(DXBITS)+XTRA-1):0] unused;
	assign	unused = {
			lprod[(DXBITS-1):0],
			r_value[(XTRA-1):0],
			qprod[(DXBITS-1):0] };
	// verilator lint_on  UNUSED

endmodule

////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	sintable.h
// {{{
// Project:	A series of CORDIC related projects
//
// Purpose:	To define the inerface associated with separate different
//		table-based sinewave calculators that can be used within an
//	FPGA.  This routine not only creates a table based sinewave calculator,
//	but also creates a hex file defining the values in the table that can
//	be used.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2017-2024, Gisselquist Technology, LLC
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
// }}}
// License:	GPL, v3, as defined and found on www.gnu.org,
// {{{
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
// }}}
#ifndef	SINTABLE_H
#define	SINTABLE_H

#include <stdio.h>

extern	void	sintable(FILE *fp, const char *fname, const char *cmdline,
			int lgtable, int ow,
			bool with_reset, bool with_aux, bool async_reset);

extern	void	quarterwav(FILE *fp, const char *fname, const char *cmdline,
			int lgtable, int ow,
			bool with_reset, bool with_aux, bool async_reset);

#endif

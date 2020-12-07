////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	seqpolar.h
// {{{
// Project:	A series of CORDIC related projects
//
// Purpose:	
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
// }}}
// License:	GPL, v3, as defined and found on www.gnu.org,
// {{{
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
// }}}
#ifndef	SEQPOLAR_H
#define	SEQPOLAR_H

#include <stdio.h>

extern	void	seqpolar(FILE *fp, FILE *fhp, const char *cmdline,
			const char *fname,
			int nstages, int iw, int ow, int nxtra,
			int phase_bits=32,
			bool with_reset=true, bool with_aux = true,
			bool async_reset = false);

#endif	// SEQPOLAR_H

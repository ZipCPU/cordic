////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	quadtbl.h
// {{{
// Project:	A series of CORDIC related projects
//
// Purpose:	This .h file notes the default parameter values from
//		within the generated file.  It is used to communicate
//	information about the design to the bench testing code.
//
// This core was generated via a core generator using the following command
// line:
//
//  % (Not given)
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
#ifndef	QUADTBL_H
#define	QUADTBL_H
const	int	OW         = 13; // bits
const	int	NEXTRA     = 3; // bits
const	int	PW         = 18; // bits
const	long	TBL_LGSZ  = 6; // (Units)
const	long	TBL_SZ    = 64; // (Units)
const	long	SCALE     = 4094; // (Units)
const	double	ITBL_ERR  = -0.25; // (OW Units)
const	double	TBL_ERR   = -0.0000037981536051; // (sin Units)
const	double	SPURDB    = -107.97; // dB
const	bool	HAS_RESET = true;
const	bool	HAS_AUX   = true;
#define	HAS_RESET_WIRE
#define	HAS_AUX_WIRES
#endif	// QUADTBL_H

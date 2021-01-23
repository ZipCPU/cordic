////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	seqpolar.h
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
// Copyright (C) 2017-2021, Gisselquist Technology, LLC
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
#ifndef	SEQPOLAR_H
#define	SEQPOLAR_H
#ifdef	CLOCKS_PER_OUTPUT
#undef	CLOCKS_PER_OUTPUT
#endif	// CLOCKS_PER_OUTPUT
#define	CLOCKS_PER_OUTPUT	21
const int	IW = 13;
const int	OW = 13;
const int	NEXTRA = 4;
const int	WW = 21;
const int	PW = 21;
const int	NSTAGES = 18;
const double	QUANTIZATION_VARIANCE = 0.1964179315931617; // (Units^2)
const double	PHASE_VARIANCE_RAD = 0.0000000000669195; // (Radians^2)
const double	GAIN = 0.8233801290585359;
const bool	HAS_RESET = true;
const bool	HAS_AUX   = true;
#define	HAS_RESET_WIRE
#define	HAS_AUX_WIRES
#endif	// SEQPOLAR_H

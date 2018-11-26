////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	cordic.h
//
// Project:	A series of CORDIC related projects
//
// Purpose:	This .h file notes the default parameter values from
//		within the generated file.  It is used to communicate
//	information about the design to the bench testing code.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017-2018, Gisselquist Technology, LLC
//
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
//
#ifndef	CORDIC_H
#define	CORDIC_H
const int	IW = 12;
const int	OW = 12;
const int	NEXTRA = 3;
const int	WW = 15;
const int	PW = 19;
const int	NSTAGES = 15;
const double	QUANTIZATION_VARIANCE = 2.7504e-01; // (Units^2)
const double	PHASE_VARIANCE_RAD = 8.7713e-10; // (Radians^2)
const double	GAIN = 1.1644353453251708;
const double	BEST_POSSIBLE_CNR = 72.98;
const bool	HAS_RESET = true;
const bool	HAS_AUX   = true;
#define	HAS_RESET_WIRE
#define	HAS_AUX_WIRES
#endif	// CORDIC_H

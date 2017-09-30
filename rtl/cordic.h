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
// Copyright (C) 2017, Gisselquist Technology, LLC
//
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of  the GNU General Public License as published
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
#ifndef	CORDIC_H
#define	CORDIC_H
const int	IW = 8;
const int	OW = 8;
const int	NEXTRA = 5;
const int	WW = 13;
const int	PW = 17;
const int	NSTAGES = 13;
const double	QUANTIZATION_VARIANCE = 2.0059e-01; // (Units^2)
const double	PHASE_VARIANCE_RAD = 8.8368e-09; // (Radians^2)
const double	GAIN = 1.1644353426140084;
const double	BEST_POSSIBLE_CNR = 50.37;
const bool	HAS_RESET = true;
const bool	HAS_AUX   = true;
#define	HAS_RESET_WIRE
#define	HAS_AUX_WIRES
#endif	// CORDIC_H

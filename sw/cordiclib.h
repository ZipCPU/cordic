////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	cordiclib.h
//
// Project:	A series of CORDIC related projects
//
// Purpose:	Defines a series of helper or library routines which may be
// 		used by all of the CORDIC modules.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017-2019, Gisselquist Technology, LLC
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
#ifndef	CORDICLIB_H
#define	CORDICLIB_H

#include <stdio.h>

extern	int	nextlg(unsigned);
extern	double	cordic_gain(int nstages);
extern	double	phase_variance(int nstages, int phase_bits);
extern	double	transform_quantization_variance(int nstages, int xtrabits, int dropped_bits);
extern	void	cordic_angles(FILE *fp, int nstages, int phase_bits, bool mem = false);
extern	int	calc_stages(const int working_width, const int phase_bits);
extern	int	calc_stages(const int phase_bits);
extern	int	calc_phase_bits(const int output_width);

#endif

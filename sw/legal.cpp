////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	legal.cpp
//
// Project:	A series of CORDIC related projects
//
// Purpose:	To insert a legal copywrite statement at the top of every file
//		called.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017-2020, Gisselquist Technology, LLC
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
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "legal.h"

const	char	PROJECT[] = "A series of CORDIC related projects";

void	legal(FILE *fp, const char *fname, const char *project,
	const char *purpose) {
fprintf(fp,
"////////////////////////////////////////////////////////////////////////////////\n"
"//\n"
"// Filename: 	%s\n"
"//\n"
"// Project:	%s\n"
"//\n"
"// Purpose:	%s\n"
"//\n"
"// Creator:	Dan Gisselquist, Ph.D.\n"
"//		Gisselquist Technology, LLC\n"
"//\n"
"////////////////////////////////////////////////////////////////////////////////\n"
"//\n"
"// Copyright (C) 2017-2020, Gisselquist Technology, LLC\n"
"//\n"
"// This file is part of the CORDIC related project set.\n"
"//\n"
"// The CORDIC related project set is free software (firmware): you can\n"
"// redistribute it and/or modify it under the terms of the GNU Lesser General\n"
"// Public License as published by the Free Software Foundation, either version\n"
"// 3 of the License, or (at your option) any later version.\n"
"//\n"
"// The CORDIC related project set is distributed in the hope that it will be\n"
"// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"// MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser\n"
"// General Public License for more details.\n"
"//\n"
"// You should have received a copy of the GNU Lesser General Public License\n"
"// along with this program.  (It's in the $(ROOT)/doc directory.  Run make\n"
"// with no target there if the PDF file isn't present.)  If not, see\n"
// <http://www.gnu.org/licenses/> for a copy.\n"
""//\n"
"// License:	LGPL, v3, as defined and found on www.gnu.org,\n"
"//		http://www.gnu.org/licenses/lgpl.html\n"
"//\n"
"////////////////////////////////////////////////////////////////////////////////\n"
"//\n"
"//\n", fname, project, purpose);
}

char *modulename(const char *fname) {
	const	char	*cptr;
		char	*ptr;
	int	slen;

	cptr = strrchr(fname, '/');
	if (NULL == cptr)
		ptr = strdup(fname);
	else
		ptr = strdup(cptr+1);

	slen = strlen(ptr);
	if ((slen > 2)&&(0 == strcmp(&ptr[slen-2], ".v")))
		ptr[slen-2] = '\0';
	return ptr;
}


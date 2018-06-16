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
// Copyright (C) 2017-2018, Gisselquist Technology, LLC
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
"// Copyright (C) 2017-2018, Gisselquist Technology, LLC\n"
"//\n"
"// This program is free software (firmware): you can redistribute it and/or\n"
"// modify it under the terms of the GNU General Public License as published\n"
"// by the Free Software Foundation, either version 3 of the License, or (at\n"
"// your option) any later version.\n"
"//\n"
"// This program is distributed in the hope that it will be useful, but WITHOUT\n"
"// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or\n"
"// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License\n"
"// for more details.\n"
"//\n"
"// You should have received a copy of the GNU General Public License along\n"
"// with this program.  (It's in the $(ROOT)/doc directory.  Run make with no\n"
"// target there if the PDF file isn't present.)  If not, see\n"
"// <http://www.gnu.org/licenses/> for a copy.\n"
"//\n"
"// License:	GPL, v3, as defined and found on www.gnu.org,\n"
"//		http://www.gnu.org/licenses/gpl.html\n"
"//\n"
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


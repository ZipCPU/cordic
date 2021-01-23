////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	hexfile.cpp
//
// Project:	A series of CORDIC related projects
//
// Purpose:
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017-2021, Gisselquist Technology, LLC
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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

const	char	*DEFAULT_EXTENSION = ".hex";

void	hextable(const char *fname, const int lgtable, const int ow,
		const long *data, const char *extension) {
	FILE	*hexfp;
	char	*hexfname;

	if (ow >= 31) {
		printf("Internal err: output width too large for internal data type");
		assert(ow < 31);
	}

	if (lgtable < 2) {
		printf("Internal err: Hex-table size should be larger than 4 entries\n");
		assert(lgtable >= 2);
	}

	// Append .hex to the filename
	int slen = strlen(fname);
	hexfname = new char [strlen(fname)+strlen(extension)+3];
	strcpy(hexfname, fname);
	if ((slen>4)&&(hexfname[slen-2]=='.'))
		strcpy(&hexfname[slen-2], extension);
	else
		strcat(hexfname, extension);

	// Open our file
	hexfp = fopen(hexfname, "w");
	if (NULL == hexfp) {
		fprintf(stderr, "ERR: Cannot open %s for writing\n",
			hexfname);
	} else {
		// Write the entriess to it.
		int	tbl_entries = (1<<lgtable), nc = (ow+3)/4;
		long	msk = (1l<<ow)-1l;

		for(int k=0; k<tbl_entries; k++) {
			if (data[k] > 0)
				assert(data[k] <= msk);
			else
				assert(data[k] >= -msk-1);
			if (0 == (k%8))
				fprintf(hexfp, "%s@%08x ", (k!=0)?"\n":"", k);
			fprintf(hexfp, "%0*lx ", nc, data[k] & msk);
		} fprintf(hexfp, "\n");
	}

	fclose(hexfp);
	delete[] hexfname;
}

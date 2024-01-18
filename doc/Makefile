################################################################################
##
## Filename: 	Makefile
## {{{
## Project:	A series of CORDIC related projects
##
## Purpose:	To coordinate the build of documentation PDFs from their
##		LaTeX sources.
##
##	Targets include:
##		all		Builds all documents
##
##		lgpl-3.0.pdf	Builds the LGPL license these files are released
##					under.
##
## Creator:	Dan Gisselquist, Ph.D.
##		Gisselquist Technology, LLC
##
################################################################################
## }}}
## Copyright (C) 2017-2024, Gisselquist Technology, LLC
## {{{
## This program is free software (firmware): you can redistribute it and/or
## modify it under the terms of the GNU General Public License as published
## by the Free Software Foundation, either version 3 of the License, or (at
## your option) any later version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program.  (It's in the $(ROOT)/doc directory.  Run make with no
## target there if the PDF file isn't present.)  If not, see
## <http://www.gnu.org/licenses/> for a copy.
## }}}
## License:	GPL, v3, as defined and found on www.gnu.org,
## {{{
##		http://www.gnu.org/licenses/gpl.html
##
################################################################################
##
## }}}
all:	pdf
pdf:	lgpl gpl
DSRC := src

LGPL=lgpl-3.0
.PHONY: lgpl
lgpl: $(LGPL).pdf

$(LGPL).pdf: $(DSRC)/$(LGPL).tex
	latex $(DSRC)/$(LGPL).tex
	latex $(DSRC)/$(LGPL).tex
	dvips -q -z -t letter -P pdf -o $(LGPL).ps $(LGPL).dvi
	ps2pdf -dAutoRotatePages=/All $(LGPL).ps $(LGPL).pdf
	rm $(LGPL).dvi $(LGPL).log $(LGPL).aux $(LGPL).ps

GPL=gpl-3.0
.PHONY: gpl
gpl: $(GPL).pdf

$(GPL).pdf: $(DSRC)/$(GPL).tex
	latex $(DSRC)/$(GPL).tex
	latex $(DSRC)/$(GPL).tex
	dvips -q -z -t letter -P pdf -o $(GPL).ps $(GPL).dvi
	ps2pdf -dAutoRotatePages=/All $(GPL).ps $(GPL).pdf
	rm $(GPL).dvi $(GPL).log $(GPL).aux $(GPL).ps

.PHONY: clean
clean:
	rm -f $(DSRC)/*.dvi $(DSRC)/*.log
	rm -f $(DSRC)/*.aux $(DSRC)/*.toc
	rm -f $(DSRC)/*.lot $(DSRC)/*.lof
	rm -f $(DSRC)/*.out spec.ps spec.pdf
	rm -f $(LGPL).pdf $(GPL).pdf


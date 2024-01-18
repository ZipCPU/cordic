################################################################################
##
## Filename:	sw/Makefile
## {{{
## Project:	A series of CORDIC related projects
##
## Purpose:	This file directs the build of a variety of Verilog cordic
##		demonstration files:
## }}}
## Targets:
## {{{
##	all:	Builds the cordic generator program, gencordic, and uses it to
##		build two demonstration files in the rtl/ directory: cordic.v
##		and topolar.v
##
##	clean:	Cleans up all of the build products, so that you can start
##		over from scratch.
##
##	gencordic:	Builds a cordic generation program--the main program
##		build with these instructions
##
##	topolar: Builds a rectangular to polar converter in the rtl/ directory
##
##	basiccordic: Builds a polar to rectangular converter slash exponential
##		multiplier, and places it in the rtl/ directory
##
##	quadtbl: Builds a sine-wave calculator based upon a quadratic table
##		interpolation
##
##	depends:	Caclulates dependencies, places a dependency file into
##		the obj-pc sub-directory
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
all:
## {{{
CXX  := g++
OBJDIR := obj-pc
VSRCD  := ../rtl
SOURCES:= main.cpp legal.cpp basiccordic.cpp topolar.cpp \
	sintable.cpp quadtbl.cpp hexfile.cpp seqcordic.cpp seqpolar.cpp \
	cordiclib.cpp
HEADERS:= $(wildcard $(subst .cpp,.h,$(SOURCES)))
OBJECTS:= $(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(SOURCES)))
VSRC   := topolar.v cordic.v sintable.v quarterwav.v quadtbl.v	\
	seqcordic.v seqpolar.v
CFLAGS := -g -Og -Wall
PROGRAMS:= gencordic
## }}}
all: $(PROGRAMS) $(VSRC)
## Default arguments
## {{{
INCS :=
PB   := 18	# Phase bits
NB   := 13
XTRA := 2
CRDCARGS := -vca
## }}}

## .o: Build object files
## {{{
%.o: $(OBJDIR)/%.o
$(OBJDIR)/%.o: %.cpp
	$(mk-objdir)
	$(CXX) $(CFLAGS) -c $< -o $@
## }}}

## gencordic -- the main software core-generator target
## {{{
gencordic: $(OBJECTS)
	rm -f $(VSRCD)/topolar.v
	rm -f $(VSRCD)/cordic.v
	rm -f $(VSRCD)/sintable.v
	rm -f $(VSRCD)/quarterwav.v
	rm -f $(VSRCD)/quadtbl.v
	rm -f $(VSRCD)/seqcordic.v
	rm -f $(VSRCD)/seqpolar.v
	$(CXX) $(OBJECTS) -o $@
## }}}

.PHONY: topolar topolar.v
## {{{
topolar: $(VSRCD)/topolar.v
topolar.v: topolar
$(VSRCD)/topolar.v: gencordic
	$(mk-rtldir)
	./gencordic $(CRDCARGS) -f $(VSRCD)/topolar.v -i $(NB) -o $(NB) -t r2p -x $(XTRA)
## }}}

.PHONY: seqpolar seqpolar.v
## {{{
seqpolar: $(VSRCD)/seqpolar.v
seqpolar.v: seqpolar
$(VSRCD)/seqpolar.v: gencordic
	$(mk-rtldir)
	./gencordic $(CRDCARGS) -f $(VSRCD)/seqpolar.v -i $(NB) -o $(NB) -t sr2p -x $(XTRA)
## }}}

.PHONY: basiccordic cordic.v cordic
## {{{
basiccordic: $(VSRCD)/cordic.v
cordic: basiccordic
cordic.v: basiccordic
$(VSRCD)/cordic.v: gencordic
	$(mk-rtldir)
	./gencordic $(CRDCARGS) -f $(VSRCD)/cordic.v  -v -i $(NB) -o $(NB) -t p2r -x 2 -c
## }}}

.PHONY: seqcordic cordic.v cordic
## {{{
seqcordic: $(VSRCD)/seqcordic.v
scordic: seqcordic
seqcordic.v: seqcordic
$(VSRCD)/seqcordic.v: gencordic
	$(mk-rtldir)
	./gencordic $(CRDCARGS) -f $(VSRCD)/seqcordic.v  -v -i $(NB) -o $(NB) -t sp2r -x $(XTRA) -c
## }}}

.PHONY: sintable sintable.v
## {{{
sintable: $(VSRCD)/sintable.v
sintable.v: sintable
$(VSRCD)/sintable.v: gencordic
	$(mk-rtldir)
	./gencordic $(CRDCARGS) -f $(VSRCD)/sintable.v -o $(NB) -t tbl
## }}}

.PHONY: quarterwav quarterwav.v
## {{{
quarterwav: $(VSRCD)/quarterwav.v
quarterwav.v: quarterwav
$(VSRCD)/quarterwav.v: gencordic
	$(mk-rtldir)
	./gencordic $(CRDCARGS) -f $(VSRCD)/quarterwav.v -p $(PB) -t qtr -x $(XTRA)
## }}}

.PHONY: quadtbl quadtbl.v
## {{{
quadtbl: $(VSRCD)/quadtbl.v
quadtbl.v: quadtbl
$(VSRCD)/quadtbl.v: gencordic
	$(mk-rtldir)
	./gencordic $(CRDCARGS) -f $(VSRCD)/quadtbl.v -p $(PB) -o $(NB) -t qtbl
## }}}

.PHONY: clean
## {{{
clean:
	rm -f $(PROGRAMS) tags
	rm -rf $(OBJDIR)/
	rm -f $(VSRCD)/topolar.v $(VSRCD)/cordic.v $(VSRCD)/seqcordic.v
	rm -f $(VSRCD)/sintable.v $(VSRCD)/sintable.hex
	rm -f $(VSRCD)/quarterwav.v $(VSRCD)/quarterwav.hex
	rm -f $(VSRCD)/quadtbl.v $(VSRCD)/quadtbl_ctbl.hex $(VSRCD)/quadtbl_ltbl.hex $(VSRCD)/quadtbl_qtbl.hex
## }}}

## mk-rtldir
## {{{
define	mk-rtldir
	bash -c "if [ ! -e $(VSRCD) ]; then mkdir -p $(VSRCD); fi"
endef
## }}}

## mk-objdir
## {{{
define	mk-objdir
	bash -c "if [ ! -e $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi"
endef
## }}}

## build-depends
## {{{
define	build-depends
	@echo "Building dependency file(s)"
	$(mk-objdir)
	$(CXX) $(CFLAGS) -MM $(SOURCES) > $(OBJDIR)/xdepends.txt
	@sed -e 's/^.*.o: /$(OBJDIR)\/&/' < $(OBJDIR)/xdepends.txt > $(OBJDIR)/depends.txt
	@rm $(OBJDIR)/xdepends.txt
endef
## }}}

## Tags
## {{{
tags: $(SOURCES) $(HEADERS)
	@echo "Generating tags"
	@ctags $(SOURCES) $(HEADERS)
## }}}

.PHONY: depends
## {{{
depends: tags
	$(build-depends)
## }}}

$(OBJDIR)/depends.txt: $(SOURCES) $(HEADERS)
	$(build-depends)

-include $(OBJDIR)/depends.txt

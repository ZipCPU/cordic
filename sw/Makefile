################################################################################
##
## Filename:	sw/Makefile
##
## Project:	A series of CORDIC related projects
##
## Purpose:	This file directs the build of a variety of Verilog cordic
##		demonstration files:
##
## Targets:
##
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
##
## Copyright (C) 2017, Gisselquist Technology, LLC
##
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
##
## License:	GPL, v3, as defined and found on www.gnu.org,
##		http://www.gnu.org/licenses/gpl.html
##
################################################################################
##
##
all:
CXX  := g++
OBJDIR := obj-pc
VSRCD  := ../rtl
SOURCES:= main.cpp cordiclib.cpp legal.cpp basiccordic.cpp topolar.cpp \
	sintable.cpp quadtbl.cpp hexfile.cpp
HEADERS:= $(wildcard $(subst .cpp,.h,$(SOURCES)))
OBJECTS:= $(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(SOURCES)))
VSRC   := topolar.v cordic.v sintable.v quarterwav.v quadtbl.v
CFLAGS := -g -Og -Wall
PROGRAMS:= gencordic
all: $(PROGRAMS) $(VSRC)
INCS :=

%.o: $(OBJDIR)/%.o
$(OBJDIR)/%.o: %.cpp
	$(mk-objdir)
	$(CXX) $(CFLAGS) -c $< -o $@

gencordic: $(OBJECTS)
	rm -f $(VSRCD)/topolar.v
	rm -f $(VSRCD)/cordic.v
	rm -f $(VSRCD)/sintable.v
	rm -f $(VSRCD)/quarterwav.v
	rm -f $(VSRCD)/quadtbl.v
	$(CXX) $(OBJECTS) -o $@

.PHONY: topolar topolar.v
topolar: $(VSRCD)/topolar.v
topolar.v: topolar
$(VSRCD)/topolar.v: gencordic
	$(mk-rtldir)
	./gencordic -f $(VSRCD)/topolar.v -v -i 12 -o 12 -t r2p -x 1 -c

.PHONY: basiccordic cordic.v cordic
basiccordic: $(VSRCD)/cordic.v
cordic: basiccordic
cordic.v: basiccordic
$(VSRCD)/cordic.v: gencordic
	$(mk-rtldir)
	# ./gencordic -f $(VSRCD)/cordic.v  -v -i 12 -o 12 -t p2r -x 2 -c
	./gencordic -f $(VSRCD)/cordic.v  -v -i 24 -o 24 -t p2r -x 1 -c

.PHONY: sintable sintable.v
sintable: $(VSRCD)/sintable.v
sintable.v: sintable
$(VSRCD)/sintable.v: gencordic
	$(mk-rtldir)
	./gencordic -f $(VSRCD)/sintable.v  -v -o 12 -t tbl

.PHONY: quarterwav quarterwav.v
quarterwav: $(VSRCD)/quarterwav.v
quarterwav.v: quarterwav
$(VSRCD)/quarterwav.v: gencordic
	$(mk-rtldir)
	./gencordic -f $(VSRCD)/quarterwav.v  -v -p 14 -t qtr

.PHONY: quadtbl quadtbl.v
quadtbl: $(VSRCD)/quadtbl.v
quadtbl.v: quadtbl
$(VSRCD)/quadtbl.v: gencordic
	$(mk-rtldir)
	./gencordic -f $(VSRCD)/quadtbl.v -c -v -p 26 -o 24 -t qtbl

.PHONY: clean
clean:
	rm -f $(PROGRAMS) tags
	rm -rf $(OBJDIR)/
	rm -f $(VSRCD)/topolar.v $(VSRCD)/cordic.v
	rm -f $(VSRCD)/sintable.v $(VSRCD)/sintable.hex
	rm -f $(VSRCD)/quarterwav.v $(VSRCD)/quarterwav.hex
	rm -f $(VSRCD)/quadtbl.v $(VSRCD)/quadtbl_ctbl.hex $(VSRCD)/quadtbl_ltbl.hex $(VSRCD)/quadtbl_qtbl.hex


define	mk-rtldir
	bash -c "if [ ! -e $(VSRCD) ]; then mkdir -p $(VSRCD); fi"
endef

define	mk-objdir
	bash -c "if [ ! -e $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi"
endef

define	build-depends
	@echo "Building dependency file(s)"
	$(mk-objdir)
	$(CXX) $(CFLAGS) -MM $(SOURCES) > $(OBJDIR)/xdepends.txt
	@sed -e 's/^.*.o: /$(OBJDIR)\/&/' < $(OBJDIR)/xdepends.txt > $(OBJDIR)/depends.txt
	@rm $(OBJDIR)/xdepends.txt
endef

tags: $(SOURCES) $(HEADERS)
	@echo "Generating tags"
	@ctags $(SOURCES) $(HEADERS)

.PHONY: depends
depends: tags
	$(build-depends)

$(OBJDIR)/depends.txt: $(SOURCES) $(HEADERS)
	$(build-depends)

-include $(OBJDIR)/depends.txt

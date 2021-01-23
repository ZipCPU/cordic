################################################################################
##
## Filename:	rtl/Makefile
## {{{
## Project:	A series of CORDIC related projects
##
## Purpose:	To direct the Verilator build of the CORDIC demo sources.  The
##		result is C++ code (built by Verilator), that is then built
##	(herein) into an executable library.
##
## Targets:	The default target, all, builds the target test, which includes
##		the libraries necessary for Verilator testing.
##
## Creator:	Dan Gisselquist, Ph.D.
##		Gisselquist Technology, LLC
##
################################################################################
## }}}
## Copyright (C) 2017-2021, Gisselquist Technology, LLC
## {{{
## This program is free software (firmware): you can redistribute it and/or
## modify it under the terms of  the GNU General Public License as published
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
all:	test
FBDIR := .
VDIRFB:= $(FBDIR)/obj_dir

.PHONY: test topolar cordic sintable quarterwav quadtbl
## Target pseudonymns
## {{{
test: topolar cordic sintable quarterwav quadtbl seqcordic seqpolar
topolar:    $(VDIRFB)/Vtopolar__ALL.a
cordic:     $(VDIRFB)/Vcordic__ALL.a
sintable:   $(VDIRFB)/Vsintable__ALL.a
quarterwav: $(VDIRFB)/Vquarterwav__ALL.a
quadtbl:    $(VDIRFB)/Vquadtbl__ALL.a
seqcordic:  $(VDIRFB)/Vseqcordic__ALL.a
seqpolar:   $(VDIRFB)/Vseqpolar__ALL.a
## }}}

VOBJ := obj_dir
SUBMAKE := $(MAKE) --no-print-directory --directory=$(VOBJ) -f
ifeq ($(VERILATOR_ROOT),)
VERILATOR := verilator
else
VERILATOR := $(VERILATOR_ROOT)/bin/verilator
endif
VFLAGS := -Wall -MMD --trace -cc

## Dependencies
## {{{
$(VDIRFB)/Vcordic__ALL.a: $(VDIRFB)/Vcordic.h $(VDIRFB)/Vcordic.cpp
$(VDIRFB)/Vcordic__ALL.a: $(VDIRFB)/Vcordic.mk
$(VDIRFB)/Vcordic.h $(VDIRFB)/Vcordic.cpp $(VDIRFB)/Vcordic.mk: cordic.v

$(VDIRFB)/Vtopolar__ALL.a: $(VDIRFB)/Vtopolar.h $(VDIRFB)/Vtopolar.cpp
$(VDIRFB)/Vtopolar__ALL.a: $(VDIRFB)/Vtopolar.mk
$(VDIRFB)/Vtopolar.h $(VDIRFB)/Vtopolar.cpp $(VDIRFB)/Vtopolar.mk: topolar.v

$(VDIRFB)/Vsintable__ALL.a: $(VDIRFB)/Vsintable.h $(VDIRFB)/Vsintable.cpp
$(VDIRFB)/Vsintable__ALL.a: $(VDIRFB)/Vsintable.mk
$(VDIRFB)/Vsintable.h $(VDIRFB)/Vsintable.cpp $(VDIRFB)/Vsintable.mk: sintable.v

$(VDIRFB)/Vquarterwav__ALL.a: $(VDIRFB)/Vquarterwav.h $(VDIRFB)/Vquarterwav.cpp
$(VDIRFB)/Vquarterwav__ALL.a: $(VDIRFB)/Vquarterwav.mk
$(VDIRFB)/Vquarterwav.h $(VDIRFB)/Vquarterwav.cpp $(VDIRFB)/Vquarterwav.mk: quarterwav.v

$(VDIRFB)/Vquadtbl__ALL.a: $(VDIRFB)/Vquadtbl.h $(VDIRFB)/Vquadtbl.cpp
$(VDIRFB)/Vquadtbl__ALL.a: $(VDIRFB)/Vquadtbl.mk
$(VDIRFB)/Vquadtbl.h $(VDIRFB)/Vquadtbl.cpp $(VDIRFB)/Vquadtbl.mk: quadtbl.v

$(VDIRFB)/Vseqcordic__ALL.a: $(VDIRFB)/Vseqcordic.h $(VDIRFB)/Vseqcordic.cpp
$(VDIRFB)/Vseqcordic__ALL.a: $(VDIRFB)/Vseqcordic.mk
$(VDIRFB)/Vseqcordic.h $(VDIRFB)/Vseqcordic.cpp $(VDIRFB)/Vseqcordic.mk: seqcordic.v

$(VDIRFB)/Vseqpolar__ALL.a: $(VDIRFB)/Vseqpolar.h $(VDIRFB)/Vseqpolar.cpp
$(VDIRFB)/Vseqpolar__ALL.a: $(VDIRFB)/Vseqpolar.mk
$(VDIRFB)/Vseqpolar.h $(VDIRFB)/Vseqpolar.cpp $(VDIRFB)/Vseqpolar.mk: seqpolar.v
## }}}

## Verilate
## {{{
$(VDIRFB)/V%.cpp $(VDIRFB)/V%.h $(VDIRFB)/V%.mk: $(FBDIR)/%.v
	$(VERILATOR) $(VFLAGS) $*.v
## }}}

## Build the library
## {{{
$(VDIRFB)/V%__ALL.a: $(VDIRFB)/V%.mk
	$(SUBMAKE) V$*.mk
## }}}

.PHONY: clean
## {{{
clean:
	rm -rf $(VDIRFB)/
## }}}

## Automatic dependency handling
## {{{
#
# Note Verilator's dependency created information, and include it here if we
# can
DEPS := $(wildcard $(VDIRFB)/*.d)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(DEPS),)
include $(DEPS)
endif
endif
## }}}

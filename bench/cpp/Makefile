################################################################################
##
## Filename:	bench/cpp/Makefile
##
## Project:	A series of CORDIC related projects
##
## Purpose:	This file directs the build of a Verilator-based test bench to
##		prove that the cordic generator's outputs work.  This build
##	must be called after building in bench/rtl, since it depends upon the
##	products of that build.
##
## Targets:
##
##	all:	Builds cordic_tb
##
##	clean:	Cleans up all of the build products, together with the .vcd
##		files, so you can start over from scratch.
##
##	cordic_tb:	A test bench for the basic (polar to rectangular)
##			cordic.  Prints success or failure on the last line.
##
##		This program depends upon having FFTW installed.
##
##	seqcordic_tb:	This is almost identical to cordic_tb above, save that
##			the CORDIC is of a sequential (not pipelined)
##			implementation.  It shares code with cordic_tb.
##
##	topolar_tb:	A test bench for the rectangular to polar coordinate
##			conversion form of the cordic.  Prints success or
##			failure on the last line.
##
##	seqpolar_tb:	As with seqcordic_tb, this is almost identical to
##			topolar_tb, save that this is a test of the sequential
##			implementation rather than the parallel one.
##
##	quadtbl_tb:	Test the quadratic interpolation sinewave generator.
##
##	test:	Runs all testbenches
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
all: cordic_tb topolar_tb quadtbl_tb seqcordic_tb seqpolar_tb
CXX  := g++
RTLD := ../../rtl
ROBJD:= $(RTLD)/obj_dir
VERILATOR := verilator
VERILATOR_ROOT ?= $(shell bash -c '$(VERILATOR) -V|grep VERILATOR_ROOT| head -1|sed -e " s/^.*=\s*//"')
VROOT:= $(VERILATOR_ROOT)
INCS   := -I$(VROOT)/include -I$(RTLD) -I$(ROBJD)
VSRCS  := $(VROOT)/include/verilated.cpp $(VROOT)/include/verilated_vcd_c.cpp
TBOBJ  := $(ROBJD)/Vcordic__ALL.a
STBOBJ := $(ROBJD)/Vseqcordic__ALL.a
PLOBJ  := $(ROBJD)/Vtopolar__ALL.a
SPLOBJ := $(ROBJD)/Vseqpolar__ALL.a
QTOBJ  := $(ROBJD)/Vquadtbl__ALL.a
CFLAGS := -g -Og -Wall $(INCS) # -faligned-new

cordic_tb:	cordic_tb.cpp $(TBOBJ) $(ROBJD)/Vcordic.h testb.h fft.h fftw.c
	$(CXX) $(CFLAGS) cordic_tb.cpp fftw.c $(VSRCS) $(TBOBJ) -lfftw3 -o $@

seqcordic_tb:	cordic_tb.cpp $(STBOBJ) $(ROBJD)/Vseqcordic.h testb.h fft.h fftw.c
	$(CXX) $(CFLAGS) -D CLOCKS_PER_OUTPUT cordic_tb.cpp fftw.c $(VSRCS) $(STBOBJ) -lfftw3 -o $@

topolar_tb:	topolar_tb.cpp $(PLOBJ) $(ROBJD)/Vtopolar.h testb.h
	$(CXX) $(CFLAGS) topolar_tb.cpp $(VSRCS) $(PLOBJ) -o $@

seqpolar_tb:	topolar_tb.cpp $(SPLOBJ) $(ROBJD)/Vseqpolar.h testb.h
	$(CXX) $(CFLAGS) -DCLOCKS_PER_OUTPUT topolar_tb.cpp $(VSRCS) $(SPLOBJ) -o $@

quadtbl_tb:	quadtbl_tb.cpp $(PLOBJ) $(ROBJD)/Vquadtbl.h testb.h fft.h fftw.c
	$(CXX) $(CFLAGS) quadtbl_tb.cpp fftw.c $(VSRCS) $(QTOBJ) -lfftw3 -o $@

test:	cordic_tb topolar_tb
	./cordic_tb
	./topolar_tb
	./quadtbl_tb

clean:
	rm -f cordic_tb     topolar_tb      quadtbl_tb
	rm -f cordic_tb.vcd topolar_tb.vcd  quadtbl_tb.vcd


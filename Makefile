################################################################################
##
## Filename: 	Makefile
## {{{
## Project:	A series of CORDIC related projects
##
## Purpose:	Provides a master Makefile for the project, coordinating the
##		build of the core generator, documents, test benches, and
##	Verilog code.
##
##	The core generator is highly configurable.  You can adjust the
##	configuration parameters in the sw/Makefile, or run it outside of any
##	Makefile context.
##
##	Targets include:
##		all		Builds everything
##		clean		Removes all build products
##		doc		Builds the documentation (licenses)
##		rtl		Runs Verilator on the Verilog files
##		sw		Builds the core generator
##		bench		Builds the bench testing software
##
## Creator:	Dan Gisselquist, Ph.D.
##		Gisselquist Technology, LLC
##
################################################################################
## }}}
## Copyright (C) 2017-2022, Gisselquist Technology, LLC
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
##
## License:	GPL, v3, as defined and found on www.gnu.org,
##		http://www.gnu.org/licenses/gpl.html
##
################################################################################
##
## }}}
.PHONY: all clean doc rtl sw bench
all:	sw rtl bench
SUBMAKE := make --no-print-directory -C

sw:
	$(SUBMAKE) sw

rtl: sw
	$(SUBMAKE) rtl

bench: rtl
	$(SUBMAKE) bench/cpp

test: bench
	$(SUBMAKE) bench/cpp test

clean:
	$(SUBMAKE) sw        clean
	$(SUBMAKE) rtl       clean
	$(SUBMAKE) bench/cpp clean

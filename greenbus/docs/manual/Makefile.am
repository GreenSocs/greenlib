# LICENSETEXT
# 
#   Copyright (C) 2007 : GreenSocs Ltd
#       http://www.greensocs.com/ , email: info@greensocs.com
# 
#   Developed by :
# 
#   Wolfgang Klingauf, Robert Guenzel
#     Technical University of Braunschweig, Dept. E.I.S.
#     http://www.eis.cs.tu-bs.de
# 
#   Mark Burton, Marcus Bartholomeu
#     GreenSocs Ltd
# 
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
#   02110-1301  USA
# 
# ENDLICENSETEXT
## ****************************************************************************
##
##  Makefile.am --
##  Process this file with automake to produce a Makefile.in file.
##
##
## ****************************************************************************


if DOXYGEN
doxy=html/index.html
endif
if LATEX
ps=GreenBusMan.ps
endif
if PS2PDF
pdf=GreenBusMan.pdf
endif

all-local:
	@echo "Nothing to do : tlm is just a set of includes"


# Create user manual

docsdir = @prefix@/docs
dist_docs_DATA = ${pdf} Doxyfile mainpage.doxygen 

GreenBusMan.pdf: GreenBusMan.ps
	ps2pdf -sPAPERSIZE=a4 $<

GreenBusMan.ps: GreenBusMan.dvi
	export TEXINPUTS=:.:$(srcdir);dvips $< -o $@

GreenBusMan.dvi: GreenBusMan.tex 
	export TEXINPUTS=:.:$(srcdir);latex $(srcdir)/GreenBusMan &>/dev/null
	export TEXINPUTS=:.:$(srcdir);latex $(srcdir)/GreenBusMan &>/dev/null
	export TEXINPUTS=:.:$(srcdir);latex $(srcdir)/GreenBusMan &>/dev/null


# Create the doxygen documentation (after install, cause autotools does not deal well with directories)

# Tar does not accept path+name greater then 99 chars. Some Doxygen generated names are too big.
# So, don't put in the distribuition
#dist-hook: ${doxy}
#	cp -pR $(srcdir)/html/* $(distdir)/html/

#install-data-hook: $(doxy)
html/index.html:
	cd $(srcdir) && doxygen &>/dev/null
	mv $(srcdir)/html/* html/




CLEANFILES = GreenBusMan.ps GreenBusMan.pdf GreenBusMan.aux GreenBusMan.dvi GreenBusMan.log GreenBusMan.toc

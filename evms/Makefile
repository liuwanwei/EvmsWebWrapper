# Enterprise Volume Management System
#
# (C) Copyright IBM Corp. 2003
#
# This program is free software;  you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY;  without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
# the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program;  if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

include ../make.rules
include make.rules
LD        = ld

NAME	  = evms-cli
LIBS	  = -lc -lpthread -levms
AR_TARGET = lib${NAME}.a.${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_LEVEL}
SO_TARGET = lib${NAME}.so.${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_LEVEL}
TARGET    = lib${NAME}.so

all: $(SO_TARGET) 

$(AR_TARGET): $(OBJECTS)
	ar -rsv $(AR_TARGET) $(OBJECTS)
	rm $(TARGET) -f
	ln -s $(AR_TARGET) $(TARGET)

$(SO_TARGET): $(OBJECTS)
	$(LD) -shared -o $(SO_TARGET) $(LIBS) $(OBJECTS) 
	cp $(SO_TARGET) $(LIB_DIR)$(SO_TARGET) -rf
	rm $(LIB_DIR)$(TARGET) -f
	ln -s $(LIB_DIR)$(SO_TARGET) $(LIB_DIR)$(TARGET)

install: 
	$(INSTALL) -m 755 $(TARGET) $(DESTDIR)$(sbindir)

uninstall:
	rm -f $(DESTDIR)$(sbindir)/$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) ${AR_TARGET} ${SO_TARGET} a.out

distclean: clean
	rm -f Makefile


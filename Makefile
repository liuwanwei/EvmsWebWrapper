# -- $Id: Makefile,v 1.7.2.4.4.9 2004/08/13 00:56:27 yuzhe Exp $ -- #

CPPFLAGS := -g -Wall -DDEBUG 
EVMS_LIB := libevms-cli.so
# PACK_LIB := libmypack.so
TARGET   := net_server client
OBJECTS  := Main.o NetServer.o LinkCtrl.o SysCheckInCmd.o RecvOneFrame.o \
	    DeadThreadTbl.o CtrlLinkTbl.o Functions.o\
            NetLog.o  UnknownRequest.o HandShakeCmd.o EvmsCmd.o \
	    InfinibandCmd.o IscsiCmd.o SystemCmd.o BondingCmd.o

all:  RULES $(EVMS_LIB) $(PACK_LIB) ${TARGET}
	@chmod +x shell/*.sh

RULES:
	@./configure.sh

$(EVMS_LIB):
	make -C evms

$(PACK_LIB):
	make -C pack

net_server : $(OBJECTS)
	gcc $(CPPFLAGS) $(OBJECTS) -o $@  -lpthread -levms-cli 

#@chmod -f +s $@

client:	Client.o PrintObject.o evms/ObjectStructs.o evms/ReplacedStructs.o evms/RevertStructs.o evms/PrivateData.o
	gcc -g -Wall -o $@ $^ 

%.o: %.c
	gcc -g -Wall -I/usr/include/evms -c $<

clean :
	make -C evms clean
	#make -C pack clean
	@rm -rf *.o ${TARGET}
	@rm -rf ib/*.o 

install:
	@cp net_server /usr/sbin/sanager/ -rf
	@cp shell/* /usr/sbin/sanager/ -rf
	@cp pl/*.pl /usr/sbin/sanager/ -rf
	make -C evms install

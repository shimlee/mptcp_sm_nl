#/***********************************************************************
#
#     Name:     PRJ Kernel Module Makefile
#
#     Type:     make file
#
#     Desc:     Compile, assemble and link product Naucra SW.
#
#     File:     Makefile
#
#     Sid:      Makefile
#
#     Prg:      frau / comate
#
#***********************************************************************/

#=======================================================================
# SECTION 1
# Defines to specify appropriate directories
#KVER := `uname -r`
#KDIR   := /usr/src/kernels/$(KVER)
#KDIR   := /home/nkernels/$(KVER)
#KVER   := mptcp.work
KVER   := current
KDIR   := /home/magw/kernels/$(KVER)
INSTALL_DIR=$(PRJ_HOME)/bin
KBUILD_EXTRA_SYMBOLS := $(KDIR)
#=======================================================================
# SECTION 2
# Product specific options
#CFLAGS_MODULE=-Werror -D__KERNEL__ -DMODULE -D_INTERNAL_TEST_
CFLAGS_MODULE=-Werror -D__KERNEL__ -DMODULE 

MNAME  := mptcp_sm_nl
MEXT   := ko

TARGET := $(MNAME).$(MEXT)


#=======================================================================
# SECTION 3
# Macros for source and object files
obj-m    := $(MNAME).o
$(MNAME)-y := nc_wd.o nc_nl.o
#$(MNAME)-objs := nc_wd.o nc_nl.o
#$(MNAME)-objs := nc_wd.o nc_nl.o



#=======================================================================
# SECTION 4
all: build

build:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	@echo "------------------------------------------"
	@echo " " $(MNAME).$(MEXT) " successfully made."
	@echo "------------------------------------------"


help:
	$(MAKE) -C $(KDIR) M=$(PWD) help
	@echo "------------------------------------------"
	@echo " " $(MNAME).$(MEXT) " successfully made."
	@echo "------------------------------------------"

ins:
	insmod $(TARGET)
	sysctl -w net.mptcp.mptcp_session_monitor=sm_nl
	@echo "------------------------------------------"
	sysctl -a |grep mptcp
	@echo "------------------------------------------"


rmm:
	rmmod ./$(MNAME).$(MEXT)
	@echo "------------------------------------------"
	sysctl -a |grep mptcp
	@echo "------------------------------------------"

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
	sysctl -w net.mptcp.mptcp_session_monitor=sm_nl
	@echo "------------------------------------------"
	@echo " " $(MNAME).$(MEXT) " successfully made."
	@echo "------------------------------------------"

install135:
	scp $(TARGET) magw@192.168.20.135:$(INSTALL_DIR)

linstall:
	scp $(TARGET) magw@192.168.20.122:/home/magw/INSTALL/MAGW

test:
	rm -rf *.o app
	gcc -Wall -g -o app app.c -lnl -lpthread

# clean-----------------------------------------------------------------
#
# cleans up everything...               
#
clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order app
	$(MAKE) -C $(KDIR) M=$(PWD) clean

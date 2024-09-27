.PHONY: all
 
all: KMC SDP PCC KMCPCC STEST clean
 
 
include conf.config
 
 
ifeq ($(strip $(PLATFORM_LINUX)), y)
cbb_platform := linux
endif
 
ifeq ($(strip $(LIB_BIT32)), y)
cbb_bits := 32
else
cbb_bits := 64
endif
 
ifeq ($(strip $(LIBTYPE_SHARED)), y)
cbb_libtype := shared
else
cbb_libtype := static
endif
 
KMC:
	$(MAKE) -f makefile_imp  libtype=$(cbb_libtype) ordtype=KMC
 
SDP:
	$(MAKE) -f makefile_imp  libtype=$(cbb_libtype) ordtype=KMC
	$(MAKE) -f makefile_imp  libtype=$(cbb_libtype) ordtype=SDP
	
PCC:
	$(MAKE) -f makefile_imp  libtype=$(cbb_libtype) ordtype=PCC
	
KMCPCC:
	$(MAKE) -f makefile_imp  libtype=$(cbb_libtype) ordtype=KMCPCC
	
STEST:
	$(MAKE) -f makefile_imp  libtype=exe ordtype=STEST
	
clean:
	$(MAKE) -f makefile_imp clean libtype=$(cbb_libtype) ordtype=KMC
	$(MAKE) -f makefile_imp clean libtype=$(cbb_libtype) ordtype=SDP
	
export cbb_platform cbb_bits cbb_libtypes
export CFLAGS CXXFLAGS
export INCLUDE_PATH VPATH WORK_DIR LIBRARY_PATH LIBRARY_NAME
export CC CXX AR LD NM
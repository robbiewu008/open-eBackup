# IOMirror Module Makefile
obj-m := iomirror.o
iomirror-objs := module.o
iomirror-objs += filter.o
# iomirror-objs += bitmap.o
iomirror-objs += protect.o
iomirror-objs += util.o
iomirror-objs += ioctl.o
iomirror-objs += backup_ioctl.o
iomirror-objs += handle_ext_cmd.o
iomirror-objs += bitmap_alloc.o
iomirror-objs += ../share/om_bitmap.o

iomirror-objs += secureprintoutput_a.o
iomirror-objs += securecutil.o
iomirror-objs += memset_s.o
iomirror-objs += memcpy_s.o
iomirror-objs += sprintf_s.o
iomirror-objs += snprintf_s.o
iomirror-objs += vsprintf_s.o
iomirror-objs += vsnprintf_s.o
iomirror-objs += strncpy_s.o

PWD=$(shell pwd)
KDIR=/lib/modules/$(shell uname -r)/build

EXTRA_CFLAGS += -DSUPPORT_BACKUP
#EXTRA_CFLAGS += -DDBG_OM_BITMAP
EXTRA_CFLAGS += -DDEBUG
EXTRA_CFLAGS += -DLINUX_KERNEL
EXTRA_CFLAGS += -I$(PWD)/../../../inc/driver/linux
EXTRA_CFLAGS += -I$(PWD)/../../../inc/driver/share

EXTRA_CFLAGS += -I$(PWD)/../../../../platform/securec/include
EXTRA_CFLAGS += -I$(PWD)/../../../../platform/securec/src

default:
	$(MAKE) -C $(KDIR) M=$(PWD) 'EXTRA_CFLAGS=$(EXTRA_CFLAGS)' modules
	
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f $(PWD)/../share/om_bitmap.o
	

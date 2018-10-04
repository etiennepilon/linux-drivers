#ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= pilote_test.o

#else

#KERNELDIR ?= /lib/modules/$(shell uname -r)/build
#PWD       := $(shell pwd)

#modules:
#	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

#endif



#clean:
#	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

#depend .depend dep:
#	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -M *.c > .depend


#ifeq (.depend,$(wildcard .depend))
#include .depend
#endif

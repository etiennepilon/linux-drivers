# -- Variables --

KERNEL_DIRS 	:=	/lib/modules/$(shell uname -r)/build
PWD				:=  $(shell pwd)

SRCS = charDriver.c circularBuffer.c
OBJS = $(SRCS:.c=.o)
obj-m := lab1.o
lab1-objs := $(OBJS)

all:
	$(MAKE) -C $(KERNEL_DIRS) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIRS) M=$(PWD) clean
	$(RM) Module.markers modules.order


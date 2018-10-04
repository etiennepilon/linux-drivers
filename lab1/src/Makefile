# -- Variables --

KERNEL_DIRS 	:=	/lib/modules/$(shell uname -r)/build
PWD				:=  $(shell pwd)

obj-m		= lab_1.o
lab1-objs	:= charDriver.o #Add objects here

all:
	@make -C $(KERNEL_DIRS) M=$(PWD) modules


KERNEL_DIR    	:= /lib/modules/$(shell uname -r)/build
PWD        	:= $(shell pwd)

obj-m        := usb_cam.o

all: usb_cam

usb_cam:
	@echo "Making USB CAM module"
	@make -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order usbcam_test
	-rm -rf .tmp_versions

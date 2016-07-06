ifneq ($(KERNELRELEASE),)

# Binary set for kernel build
include Kbuild

else

# normal Makefile
KVERSION = $(shell uname -r)

default:
	$(MAKE) -C /lib/modules/$(KVERSION)/build SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(KVERSION)/build SUBDIRS=$(PWD) clean

# Module Specific targets

endif

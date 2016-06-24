#obj-m += lkm_proc_ioctl_kernel.o
#obj-m += ioctl_kernel.o
obj-m := lkm.o
lkm-objs := lkm_proc_ioctl_kernel.o lkm_linked_list.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	
	

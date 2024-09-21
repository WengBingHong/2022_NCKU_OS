
TARGET_MODULE := My_proc
obj-m := $(TARGET_MODULE).o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: 
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc -Wall -pthread -g -o MT_matrix MT_matrix.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions MT_matrix
load:
	sudo insmod $(TARGET_MODULE).ko
unload:
	sudo rmmod $(TARGET_MODULE) || true > /dev/null

lsmod:
	sudo lsmod | grep $(TARGET_MODULE)

cat:
	sudo cat /proc/thread_info

kernel:
	sudo journalctl --since "1 min ago" | grep kernel

check: all
	$(MAKE) unload
	$(MAKE) load
	./MT_matrix
	$(MAKE) lsmod
	$(MAKE) cat
	$(MAKE) kernel

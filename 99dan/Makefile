#Makefile for a basic kernel module

obj-m   := gpio_driver.o

KDIR    :=/root/xstation/Kernel/linux-2.6.21-xstation
PWD     :=$(shell pwd)

all: driver app
#all: driver 

driver:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

app:
#	/usr/local/arm/gcc-4.0.2-glibc-2.3.6/arm-softfloat-linux-gnu/bin/arm-softfloat-linux-gnu-gcc -o test_gpio test_gpio.c
	arm-linux-gcc -o client client.c -lpthread

clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf *.o
	rm -rf test_gpio


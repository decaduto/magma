obj-m +=MagMV.o
CFLAGS_MagMV.o := -O3 -faggressive-loop-optimizations -Wframe-larger-than=2048
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean        
        
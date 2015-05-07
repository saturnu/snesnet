# -*-makefile-*-
.PHONY: all clean


all:
	gcc -g -O2 -Wall \
	-lconfig `mysql_config --cflags --libs` \
	-o ./vsnesnet  \
	axbtnmap.c vsnesnet.c login.c \
	 -lpthread


clean:
	rm ./vsnesnet


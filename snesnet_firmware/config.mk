# -*-makefile-*-


# Basic configuration.
BAUD    = 9600UL
PORT    = /dev/ttyUSB0


# Nothing below this line need to be changed.
F_CPU   = 16000000UL
MCU     = atmega168
OBJ     = 		src/io.o \
			src/trivium.o \
			src/main.o
PRG     = firmware

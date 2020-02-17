##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

V := 1

OPENCM3_DIR := ../../submodules/libopencm3
VPATH       := ../lib

SOURCES     := main.c dda.c ee_printf.c gpio.c l6234.c led.c math.c     \
               print-assert.c regdump.c systick.c timer.c usart.c       \
               steval-ihm043v1.c
OBJS        := $(patsubst ../lib/%,%,$(SOURCES:.c=.o))

BINARY      := open-loop
LDLIBS      := -lm
LDSCRIPT    := ../stm32f0-steval.ld

include ../Generic.make

CPPFLAGS    +=                                                          \
               -Wno-array-bounds                                        \
               -Wno-attributes                                          \
               -Wno-parentheses                                         \
               -Werror                                                  \
               -Wno-error=char-subscripts                               \
               -std=gnu11

CPPFLAGS    += -iquote ../include -DTARGET_H='"steval-ihm043v1.h"'

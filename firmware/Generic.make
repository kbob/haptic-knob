##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
## Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>
## Copyright (C) 2011 Fergus Noble <fergusnoble@gmail.com>
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

LIBNAME         = opencm3_stm32f0
DEFS            += -DSTM32F0

FP_FLAGS        ?= -mfloat-abi=soft
ARCH_FLAGS      = -mthumb -mcpu=cortex-m0 $(FP_FLAGS)

################################################################################
# Black Magic Probe specific variables
# Set the BMP_PORT to a serial port and then BMP is used for flashing
BMP_PORT        ?= /dev/cu.usbmodem7AB685971

################################################################################
# texane/stlink specific variables
#STLINK_PORT    ?= :4242

include ../Rules.make

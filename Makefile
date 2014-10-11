# Makefile for Lab03-Blinky, ELEC424 Fall 2014. To run on the CrazyFlie
# Authors: Jeremy Hunt, Christopher Buck. Rice University 
# Date: 9-30-14
# Derived from the crazyflie-firmware Makefile

# Path Definitions
# This path should point to the STM32 Library directory
STM_LIB ?= local_install/symlinks/stm_std_libs/

# Project Directories
INCDIR = inc
BINDIR = bin
BUILDDIR = build
SRCDIR = src
LINKER_SCRIPT = linker_script

# Compiler 
CC = arm-none-eabi-gcc

# Particular processor
PROCESSOR = -mcpu=cortex-m3 -mthumb

# STM specific info
STFLAGS = -DSTM32F10X_MD
STM_BUILD_DIR := $(BUILDDIR)/stm_lib
STM_CORE = /CMSIS/CM3/CoreSupport/
STM_DEVICE_CORE = /CMSIS/CM3/DeviceSupport/ST/STM32F10x/
STM_DEVICE_PERIPH = /STM32F10x_StdPeriph_Driver/

# These libary sources will be built into the STM libary for linking
STM_LIB_PKG = $(BINDIR)/libstm32f10x.a
STM_INCLUDES = $(addprefix -I$(STM_LIB), $(STM_CORE) $(STM_DEVICE_CORE) $(STM_DEVICE_PERIPH)/inc/) -include $(INCDIR)/stm32f10x_conf.h
STM_SOURCES_ASM = $(STM_LIB)/$(STM_DEVICE_CORE)/startup/gcc_ride7/startup_stm32f10x_md.s
STM_SOURCES_C = $(STM_LIB)/$(STM_DEVICE_CORE)/system_stm32f10x.c \
	$(addprefix $(STM_LIB)/$(STM_DEVICE_PERIPH)/src/, stm32f10x_gpio.c stm32f10x_rcc.c stm32f10x_tim.c misc.c)
STM_OBJS := $(patsubst $(STM_LIB)/%,$(STM_BUILD_DIR)/%,$(STM_SOURCES_C:.c=.o) $(STM_SOURCES_ASM:.s=.o)) 
STM_DEPS := $(patsubst $(STM_LIB)/%,$(STM_BUILD_DIR)/%,$(STM_SOURCES_C:.c=.d))


# Directories of used header files
INCLUDE = -I$(INCDIR) $(STM_INCLUDES)

# Define the compiler flags
CFLAGS = -O0 -g -Wall -Wextra $(PROCESSOR) $(INCLUDE) $(STFLAGS) -Wl,--gc-sections -T $(LINKER_SCRIPT)/stm32_flash.ld

SRCS = $(SRCDIR)/blinky.c
OBJS = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRCS:.c=.o))
DEPENDENCIES := $(STM_DEPS) $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRCS:.c=.d))
ELF = $(BINDIR)/blinky.elf

# Build all relevant files and create .elf
all: $(ELF)
	
# Build source files into objects
$(STM_BUILD_DIR)/%.o: $(STM_LIB)/%.s
	@mkdir -p $(patsubst %/,%,$(dir $@)) # Create necessary dirs in build
	$(CC) $(CFLAGS) -o $@ -c $<

$(STM_BUILD_DIR)/%.o: $(STM_LIB)/%.c
	@mkdir -p $(patsubst %/,%,$(dir $@)) # Create necessary dirs in build
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(patsubst %/,%,$(dir $@)) # Create necessary dirs in build
	$(CC) $(CFLAGS) -o $@ -c $<

# Archive the STM object files into a library 
$(STM_LIB_PKG): $(STM_OBJS) 
	ar rcs $@ $^

# Link the program's object files and the libraries together into an executable elf file.
$(ELF): $(OBJS) $(STM_LIB_PKG)
	$(CC) $(CFLAGS) $^ -o $@

# Program .elf into Crazyflie flash memory via the busblaster
flash: $(ELF)
	@openocd -d0 -f interface/busblaster.cfg -f target/stm32f1x.cfg -c init -c targets -c "reset halt" -c "flash write_image erase $(ELF)" -c "verify_image $(ELF)" -c "reset run" -c shutdown

# Runs OpenOCD, opens GDB terminal, and establishes connection with Crazyflie
debug: $(ELF)
	@xterm -iconic -e openocd -f interface/busblaster.cfg -f target/stm32f1x.cfg -c 'init; targets; reset halt;' &
	@arm-none-eabi-gdb -ex 'target remote localhost:3333' $(ELF)
	@killall openocd

# Remove all files generated by target 'all'
.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
	rm -f $(BINDIR)/*.a
	rm -f $(BINDIR)/*.elf

# This clever bit of make-fu builds dependency files for each source file so
# that if the included files for that source file are updated, the object for
# that file is also rebuilt. This rule generates a coresponding %.d file in
# the build directory for each source file. These %.d files contain makefile
# dependency rules which are included at the bottom of this makefile. Even
# more clever, this rule adds the %.d file as a target in addition to the %.o
# file for the rule that is generated for each source file, so that if one of
# the headers, or the source file changes, the dependency file is also
# rebuilt. This covers all of the necessary dependencies.
# Jeremy wrote it.
$(STM_BUILD_DIR)/%.d: $(STM_LIB)/%.c
	@mkdir -p $(patsubst %/,%,$(dir $@)) # Create necessary dirs in build
	$(CC) $(CFLAGS) -M -MF"$@" -MT"$@" -MT"$(@:.d=.o)" "$<"	

$(BUILDDIR)/%.d: $(SRCDIR)/%.c
	@mkdir -p $(patsubst %/,%,$(dir $@)) # Create necessary dirs in build
	$(CC) $(CFLAGS) -M -MF"$@" -MT"$@" -MT"$(@:.d=.o)" "$<"

# This include will fail at first if one of the needed %.d files does not yet
# exist, but this is NOT a problem, because 
include $(DEPENDENCIES)



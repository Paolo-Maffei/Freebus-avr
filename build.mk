#########  AVR Project Makefile Template   #########
######                                        ######
######    Copyright (C) 2003-2005,Pat Deegan, ######
######            Psychogenic Inc             ######
######          All Rights Reserved           ######
######                                        ######
###### You are free to use this code as part  ######
###### of your own applications provided      ######
###### you keep this copyright notice intact  ######
###### and acknowledge its authorship with    ######
###### the words:                             ######
######                                        ######
###### "Contains software by Pat Deegan of    ######
###### Psychogenic Inc (www.psychogenic.com)" ######
######                                        ######
###### If you use it as part of a web site    ######
###### please include a link to our site,     ######
###### http://electrons.psychogenic.com  or   ######
###### http://www.psychogenic.com             ######
######                                        ######
####################################################


##### This Makefile will make compiling Atmel AVR 
##### micro controller projects simple with Linux 
##### or other Unix workstations and the AVR-GCC 
##### tools.
#####
##### It supports C, C++ and Assembly source files.
#####
##### Customize the values as indicated below and :
##### make
##### make disasm 
##### make stats 
##### make hex
##### make writeflash
##### make gdbinit
##### or make clean
#####
##### See the http://electrons.psychogenic.com/ 
##### website for detailed instructions

####################################################
#####                                          #####
#####              Configuration               #####
#####                                          #####
##### Customize the values in this section for #####
##### your project. MCU, PROJECTNAME and       #####
##### PRJSRC must be setup for all projects,   #####
##### the remaining variables are only         #####
##### relevant to those needing additional     #####
##### include dirs or libraries and those      #####
##### who wish to use the avrdude programmer   #####
#####                                          #####
##### See http://electrons.psychogenic.com/    #####
##### for further details.                     #####
#####                                          #####
####################################################

##### Flags ####

# HEXFORMAT -- format for .hex file output
HEXFORMAT=ihex

# compiler
CFLAGS= $(CUSTOM_CFLAGS) -I. $(INC) -mmcu=$(MCU) -O$(OPTLEVEL) \
	-fpack-struct -fshort-enums             \
	-funsigned-bitfields -funsigned-char    \
	-Wall -Wstrict-prototypes               \
	-Wa,-ahlms=$(firstword                  \
	$(filter %.lst, $(<:.c=.lst)))

# c++ specific flags
CPPFLAGS=-fno-exceptions               \
	-Wa,-ahlms=$(firstword         \
	$(filter %.lst, $(<:.cpp=.lst))\
	$(filter %.lst, $(<:.cc=.lst)) \
	$(filter %.lst, $(<:.C=.lst)))

# assembler
ASMFLAGS =-I. $(INC) -mmcu=$(MCU)        \
	-x assembler-with-cpp            \
	-Wa,-gstabs,-ahlms=$(firstword   \
		$(<:.S=.lst) $(<.s=.lst))


# linker
LDFLAGS=-Wl,-Map,$(TRG).map -mmcu=$(MCU) \
	-lm $(LIBS)

##### executables ####
CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
SIZE=avr-size
AVRDUDE=avrdude
REMOVE=rm -f

##### automatic target names ####
TRG=$(PROJECTNAME).out
DUMPTRG=$(PROJECTNAME).s

HEXROMTRG=$(PROJECTNAME).hex 
HEXTRG=$(HEXROMTRG) $(PROJECTNAME).ee.hex
GDBINITFILE=gdbinit-$(PROJECTNAME)

# Define all object files.

# Start by splitting source files by type
#  C++
CPPFILES=$(filter %.cpp, $(PRJSRC))
CCFILES=$(filter %.cc, $(PRJSRC))
BIGCFILES=$(filter %.C, $(PRJSRC))
#  C
CFILES=$(filter %.c, $(PRJSRC))
#  Assembly
ASMFILES=$(filter %.S, $(PRJSRC))


# List all object files we need to create
OBJDEPS=$(CFILES:.c=.o)    \
	$(CPPFILES:.cpp=.o)\
	$(BIGCFILES:.C=.o) \
	$(CCFILES:.cc=.o)  \
	$(ASMFILES:.S=.o)

# Define all lst files.
LST=$(filter %.lst, $(OBJDEPS:.o=.lst))

# All the possible generated assembly 
# files (.s files)
GENASMFILES=$(filter %.s, $(OBJDEPS:.o=.s)) 


.SUFFIXES : .c .cc .cpp .C .o .out .s .S \
	.hex .ee.hex .h .hh .hpp


.PHONY: writeflash clean stats gdbinit stats

# Make targets:
# all, disasm, stats, hex, writeflash/install, clean
all: $(TRG)

disasm: $(DUMPTRG) stats

stats: $(TRG)
	$(OBJDUMP) -h $(TRG)
	$(SIZE) $(TRG) 

hex: $(HEXTRG)


writeflash: hex
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMERID)   \
	 -p $(PROGRAMMER_MCU) -P $(AVRDUDE_PORT) -e        \
	 -U flash:w:$(HEXROMTRG)

install: writeflash

$(DUMPTRG): $(TRG) 
	$(OBJDUMP) -S  $< > $@


$(TRG): $(OBJDEPS) 
	$(CC) $(OBJDEPS) $(LDFLAGS) -o $(TRG)


#### Generating assembly ####
# asm from C
%.s: %.c
	$(CC) -S $(CFLAGS) $< -o $@

# asm from (hand coded) asm
%.s: %.S
	$(CC) -S $(ASMFLAGS) $< > $@


# asm from C++
.cpp.s .cc.s .C.s :
	$(CC) -S $(CFLAGS) $(CPPFLAGS) $< -o $@



#### Generating object files ####
# object from C
.c.o: 
	$(CC) $(CFLAGS) -c $< -o $@


# object from C++ (.cc, .cpp, .C files)
.cc.o .cpp.o .C.o :
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# object from asm
.S.o :
	$(CC) $(ASMFLAGS) -c $< -o $@


#### Generating hex files ####
# hex files from elf
#####  Generating a gdb initialisation file    #####
.out.hex:
	$(OBJCOPY) -j .text                    \
		-j .data                       \
		-O $(HEXFORMAT) $< $@

.out.ee.hex:
	$(OBJCOPY) -j .eeprom                  \
		--change-section-lma .eeprom=0 \
		-O $(HEXFORMAT) $< $@


#####  Generating a gdb initialisation file    #####
##### Use by launching simulavr and avr-gdb:   #####
#####   avr-gdb -x gdbinit-myproject           #####
gdbinit: $(GDBINITFILE)

$(GDBINITFILE): $(TRG)
	@echo "file $(TRG)" > $(GDBINITFILE)
	@echo "target remote localhost:1212" \
		                >> $(GDBINITFILE)
	@echo "load"        >> $(GDBINITFILE) 
	@echo "break main"  >> $(GDBINITFILE)
	@echo "continue"    >> $(GDBINITFILE)
	@echo
	@echo "Use 'avr-gdb -x $(GDBINITFILE)'"


#### Cleanup ####
clean:
	$(REMOVE) $(TRG) $(TRG).map $(DUMPTRG)
	$(REMOVE) $(OBJDEPS)
	$(REMOVE) $(LST) $(GDBINITFILE)
	$(REMOVE) $(GENASMFILES)
	$(REMOVE) $(HEXTRG)


#####                    EOF                   #####

# DO NOT DELETE
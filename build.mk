# HEXFORMAT -- format for .hex file output
HEXFORMAT=ihex

ifeq ($(.DEFAULT_GOAL),)
	.DEFAULT_GOAL:=all
endif

# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands

ifeq ("$(origin V)", "command line")
  VERBOSE = $(V)
endif
ifndef VERBOSE
  VERBOSE = 0
endif

ifeq ($(VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

BINFORMAT=binary

# check if required variable are defined
ifeq ($(MCU),)
$(error Please define processor tpye with variable MCU in Make.config)
endif
ifeq ($(MEDIATYPE),)
$(error Please define media type with variable MEDIATYPE in Make.config)
endif
ifeq ($(REVISION),)
$(error Please define hardware revision with variable REVISION in Make.config)
endif

# compiler
ifeq ($(ARM), 1)
CUSTOM_CFLAGS+= -x c -mthumb -D__SAM4LC4C__ -Dscanf=iscanf -DARM_MATH_CM4=true -Dprintf=iprintf 
CUSTOM_CFLAGS+= -O1 -fdata-sections -ffunction-sections -mlong-calls -g3 -Wall -mcpu=cortex-m4 -c -pipe -fno-strict-aliasing -Wall
CUSTOM_CFLAGS+= -Wstrict-prototypes -Wmissing-prototypes -Werror-implicit-function-declaration -Wpointer-arith -std=gnu99 -ffunction-sections -fdata-sections
CUSTOM_CFLAGS+= -Wchar-subscripts -Wcomment -Wformat=2 -Wimplicit-int -Wmain -Wparentheses -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunused
CUSTOM_CFLAGS+= -Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wbad-function-cast -Wwrite-strings -Wsign-compare -Waggregate-return
CUSTOM_CFLAGS+= -Wmissing-declarations -Wformat -Wmissing-format-attribute -Wno-deprecated-declarations -Wpacked -Wredundant-decls -Wnested-externs -Wlong-long
CUSTOM_CFLAGS+= -Wunreachable-code -Wcast-align --param max-inline-insns-single=500 -MD -MP
CFLAGS= $(CUSTOM_CFLAGS) -I. $(INC) 		\
	-Wa,-ahlms=$(@:.o=.lst)
else
CFLAGS= $(CUSTOM_CFLAGS) -I. $(INC) -mmcu=$(MCU) -O$(OPTLEVEL) \
	-fpack-struct -fshort-enums             \
	-funsigned-bitfields -funsigned-char    \
	-Wall -Wstrict-prototypes               \
	-mcall-prologues -ffunction-sections    \
	-Wa,-ahlms=$(firstword                  \
	$(filter %.lst, $(<:.c=.lst)))
endif

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

# archiver
ARFLAGS=-rcs

# linker. You may want to specify CUSTOM_LDFLAGS in the makefile of your app
# (e.g. section start for bootloader)
LDFLAGS=$(CUSTOM_LDFLAGS) -Wl,-Map,$(TRG).map -Wl,--gc-sections -mmcu=$(MCU) \
	-lm $(LIBS)

ifndef PROJECT_DESCRIPTION
$(info Work on $(PROJECTNAME)...)
else
$(info Work on $(PROJECT_DESCRIPTION) (project: $(PROJECTNAME))...)
endif
##### executables ####
CROSS?=avr
CC:=$(CROSS)-gcc
OBJCOPY:=$(CROSS)-objcopy
OBJDUMP:=$(CROSS)-objdump
SIZE:=$(CROSS)-size
AR:=$(CROSS)-ar
AVRDUDE?=avrdude
REMOVE?=rm -f
INSTALL?=install
CMP?=cmp

##### automatic target names ####
TRG?=$(PROJECTNAME)_$(MCU)_$(MEDIATYPE)_$(REVISION)$(DEBUG).out
DUMPTRG=$(PROJECTNAME)$(DEBUG).s

HEXROMTRG=$(TRG:.out=.hex)
BINROMTRG=$(TRG:.out=.bin)
HEXTRG=$(HEXROMTRG:.out=.hex) $(TRG:.out=.ee.hex)
BINTRG=$(BINROMTRG)
GDBINITFILE=gdbinit-$(PROJECTNAME)$(DEBUG)

# Define all object files.

# Start by splitting source files by type
#  C++
PRJSRC?=$(DEFAULT_TARGET)
CPPFILES=$(filter %.cpp, $(PRJSRC))
CCFILES=$(filter %.cc, $(PRJSRC))
BIGCFILES=$(filter %.C, $(PRJSRC))
#  C
CFILES=$(filter %.c, $(PRJSRC))
#  Assembly
ASMFILES=$(filter %.S, $(PRJSRC))

# List all object files we need to create
OBJDEPS=$(CFILES:.c=.o) \
	$(CPPFILES:.cpp=.o) \
	$(BIGCFILES:.C=.o) \
	$(CCFILES:.cc=.o)  \
	$(ASMFILES:.S=.o)

# Define all lst files.
LST=$(filter %.lst, $(OBJDEPS:.o=.lst))

# All the possible generated assembly 
# files (.s files)
GENASMFILES=$(filter %.s, $(OBJDEPS:.o=.s)) 

# Use depedencies
ifneq ($(MAKECMDGOALS),clean)
	-include $(OBJDEPS:.o=.d)
endif

.PHONY: writeflash debug-writeflash release hex bin debug-hex debug-bin stats gdbinit all debug
.SUFFIXES : .a .o .c .h .out .hex

# check if cflags/ldflags are different to the build before
.PHONY: FORCE
compiler_flags: FORCE
	@echo '$(CFLAGS)' | $(CMP) -s - $@ || echo '$(CFLAGS)' > $@
linker_flags: FORCE
	@echo '$(LDFLAGS)' | $(CMP) -s - $@ || echo '$(LDFLAGS)' > $@

# Make targets:
# all, disasm, stats, hex, writeflash/install, clean
all: $(TRG)

debug: CUSTOM_CFLAGS+=-DDEBUG_UART -g
debug: DEBUG=debug
debug: $(TRG)
	echo $(TRG)
	
disasm: $(DUMPTRG) stats

stats: $(TRG)
	$(OBJDUMP) -h $(TRG)
	$(SIZE) $(TRG) 

hex: $(HEXTRG)

bin: $(BINTRG)

debug-hex: CUSTOM_CFLAGS+=-DDEBUG_UART -DDEBUG_LCD -g
debug-hex: DEBUG=debug
debug-hex: $(HEXTRG)

debug-bin: CUSTOM_CFLAGS+=-DDEBUG_UART -DDEBUG_LCD -g
debug-bin: DEBUG=debug
debug-bin: $(BINTRG)

writeflash: hex
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMERID)   \
	 -p $(PROGRAMMER_MCU) -P $(AVRDUDE_PORT) -e        \
	 -U flash:w:$(HEXROMTRG)

debug-writeflash: CUSTOM_CFLAGS+=-DDEBUG_UART -DDEBUG_LCD -g
debug-writeflash: DEBUG=debug
debug-writeflash: debug-hex
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMERID)   \
	 -p $(PROGRAMMER_MCU) -P $(AVRDUDE_PORT) -e        \
	 -U flash:w:$(HEXROMTRG)

release: hex
	$(info Copy $(HEXROMTRG) in $(CURDIR) to $(CURDIR)/../../releases/$(shell basename $(CURDIR))/$(HEXROMTRG))
	@mkdir -p $(CURDIR)/../../releases/$(shell basename $(CURDIR))
	@install -m 664 -p $(HEXROMTRG) $(CURDIR)/../../releases/$(shell basename $(CURDIR))/$(HEXROMTRG)
	@echo

$(DUMPTRG): $(TRG) 
	$(OBJDUMP) -S  $< > $@

$(subst .out,,$(TRG)).out: $(OBJDEPS) linker_flags
	@echo Link target $(PROJECTNAME)...
	$(Q)$(CC) $(OBJDEPS) $(LDFLAGS) -o $(TRG)

#### Generating assembly ####
# asm from C
%.s: %.c
	$(CC) -S $(CFLAGS) $< -o $@

# asm from (hand coded) asm
%.s: %.S
	$(CC) -S $(ASMFLAGS) $< > $@

# asm from C++
%.s : %.cpp %.cc %.C
	$(CC) -S $(CFLAGS) $(CPPFLAGS) $< -o $@

#### Generating object files ####
# object from C
%.o: %.c compiler_flags
	$(Q)$(CC) $(CFLAGS) -MD -MP -MF "$(@:.o=.d)" -MT"$(@:.o=.d)" -MT"$@" -c $< -o $@
#	@$(CC) -MM $(CFLAGS) -c $< -o $(@:.o=.d)

# object from C++ (.cc, .cpp, .C files)
%.o: %.cc %.cpp %.C
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
	@$(CC) -MM $(CFLAGS) $(CPPFLAGS)-c $< -o $(@:.o=.d)

# object from asm
%.o: %.S
	$(CC) $(ASMFLAGS) -c $< -o $@


#### Generating hex files ####
# hex files from elf
#####  Generating a gdb initialisation file    #####
%.hex: %.out
	@echo Build .hex for $(PROJECTNAME)...
	$(Q)$(OBJCOPY) -j .text  -j .xbootloader    \
		-j .data                       \
		-O $(HEXFORMAT) $(<:.out=$(DEBUG).out) $(@:.hex=$(DEBUG).hex)

%.bin: %.out
	@echo Build bin files
	$(Q)$(OBJCOPY) -j .text                    \
		-j .data                       \
		-O $(BINFORMAT) $(<:.out=$(DEBUG).out) $(@:.bin=$(DEBUG).bin)

%.ee.hex: %.out
	@echo Build ee.hex for $(PROJECTNAME)...
	$(Q)$(OBJCOPY) -j .eeprom                  \
		--change-section-lma .eeprom=0 \
		-O $(HEXFORMAT) $(<:.out=$(DEBUG).out) $(@:.ee.hex=$(DEBUG).ee.hex)

#####  Generating a gdb initialisation file    #####
##### Use by launching simulavr and avr-gdb:   #####
#####   avr-gdb -x gdbinit-myproject           #####
gdbinit: $(GDBINITFILE)

$(GDBINITFILE): $(TRG)
	@echo Build GDB init file
	@echo "file $(TRG)" > $(GDBINITFILE)
	@echo "target remote localhost:1212" \
		                >> $(GDBINITFILE)
	@echo "load"        >> $(GDBINITFILE) 
	@echo "break main"  >> $(GDBINITFILE)
	@echo "continue"    >> $(GDBINITFILE)
	@echo
	@echo "Use 'avr-gdb -x $(GDBINITFILE)'"


#### Cleanup ####
.PHONY: clean distclean debug-clean debug-distclean
clean:
	@echo Clean target $(PROJECTNAME)...
	$(Q)$(REMOVE) $(TRG) $(TRG).map $(DUMPTRG)
	$(Q)$(REMOVE) $(OBJDEPS)
	$(Q)$(REMOVE) $(LST) $(GDBINITFILE)
	$(Q)$(REMOVE) $(GENASMFILES)
	$(Q)$(REMOVE) $(OBJDEPS:.o=.d)
	$(Q)$(REMOVE) compiler_flags
	$(Q)$(REMOVE) linker_flags

distclean: clean
	$(Q)$(REMOVE) $(HEXTRG)
	$(Q)$(REMOVE) $(BINTRG)

debug-distclean: DEBUG=debug
debug-distclean: clean distclean

debug-clean: DEBUG=debug
debug-clean: clean

#####                    EOF                   #####

# DO NOT DELETE

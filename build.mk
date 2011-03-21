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

ifndef PROJECT_DESCRIPTION
$(info Work on $(PROJECTNAME)...)
else
$(info Work on $(PROJECT_DESCRIPTION) (project: $(PROJECTNAME))...)
endif
##### executables ####
CC:=avr-gcc
OBJCOPY:=avr-objcopy
OBJDUMP:=avr-objdump
SIZE:=avr-size
AVRDUDE:=avrdude
REMOVE:=rm -f
CMP:=cmp
##### automatic target names ####
TRG=$(PROJECTNAME)$(DEBUG).out
DUMPTRG=$(PROJECTNAME)$(DEBUG).s

HEXROMTRG=$(TRG).hex 
HEXTRG=$(HEXROMTRG) $(TRG).ee.hex
GDBINITFILE=gdbinit-$(PROJECTNAME)$(DEBUG)

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
OBJDEPS=$(CFILES:.c=.o) \
	$(CPPFILES:.cpp=.o) \
	$(BIGCFILES:.C=.o) \
	$(CCFILES:.cc=.o) \
	$(ASMFILES:.S=.o)

# Define all lst files.
LST=$(filter %.lst, $(OBJDEPS:.o=.lst))

# All the possible generated assembly 
# files (.s files)
GENASMFILES=$(filter %.s, $(OBJDEPS:.o=.s)) 

# Use depedencies
-include $(OBJDEPS:.o=.d)

.PHONY: writeflash clean distclean stats gdbinit stats all
.SUFFIXES : .o .c .h .out .hex

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

disasm: $(DUMPTRG) stats

stats: $(TRG)
	$(OBJDUMP) -h $(TRG)
	$(SIZE) $(TRG) 

hex: $(HEXTRG)

debug-hex: CUSTOM_CFLAGS+=-DDEBUG_UART -g
debug-hex: DEBUG=debug
debug-hex: $(HEXTRG)

writeflash: hex
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMERID)   \
	 -p $(PROGRAMMER_MCU) -P $(AVRDUDE_PORT) -e        \
	 -U flash:w:$(HEXROMTRG)

install: writeflash

$(DUMPTRG): $(TRG) 
	$(OBJDUMP) -S  $< > $@

$(TRG): $(OBJDEPS) linker_flags
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
	$(Q)$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) -MM $(CFLAGS) -c $< -o $(@:.o=.d)

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
%.out.hex: %.out
	@echo Build .hex for $(PROJECTNAME)...
	$(Q)$(OBJCOPY) -j .text                    \
		-j .data                       \
		-O $(HEXFORMAT) $(<:.out=$(DEBUG).out) $(@:.hex=$(DEBUG).hex)

%.out.ee.hex: %.out
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
.PHONY: clean distclean debug-clean
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

debug-distclean: DEBUG=debug
debug-distclean: clean distclean

debug-clean: DEBUG=debug
debug-clean: clean

#####                    EOF                   #####

# DO NOT DELETE

# Which target should be compiled
DEFAULT_TARGET=fb_relais_app.c

#####         Target Specific Details          #####
#####     Customize these for your project     #####

# Name of target controller 
# (e.g. 'at90s8515', see the available avr-gcc mmcu 
# options for possible values)
#MCU=atmega328p

# id to use with programmer
# default: PROGRAMMER_MCU=$(MCU)
# In case the programer used, e.g avrdude, doesn't
# accept the same MCU name as avr-gcc (for example
# for ATmega8s, avr-gcc expects 'atmega8' and 
# avrdude requires 'm8')
#PROGRAMMER_MCU=m328p

# Name of our project
# (use a single word, e.g. 'myproject')
PROJECTNAME=app-8-out
PROJECT_DESCRIPTION=8-out Application

# Source files
# List C/C++/Assembly source files:
# (list all files to compile, e.g. 'a.c b.cpp as.S'):
# Use .cc, .cpp or .C suffix for C++ files, use .S 
# (NOT .s !!!) for assembly source code files.
#PRJSRC=

# additional includes (e.g. -I/path/to/mydir)
#INC=

# libraries to link in (e.g. -lmylib)
#LIBS=

# Optimization level, 
# use s (size opt), 1, 2, 3 or 0 (off)
#OPTLEVEL=

# define additional flags for the compiler
#CUSTOM_CFLAGS=-DFB_TP

#####      AVR Dude 'writeflash' options       #####
#####  If you are using the avrdude program
#####  (http://www.bsdhome.com/avrdude/) to write
#####  to the MCU, you can set the following config
#####  options and use 'make writeflash' to program
#####  the device.


# programmer id--check the avrdude for complete list
# of available opts.  These should include stk500,
# avr910, avrisp, bsd, pony and more.  Set this to
# one of the valid "-c PROGRAMMER-ID" values 
# described in the avrdude info page.
# 
#AVRDUDE_PROGRAMMERID=

# port--serial or parallel port to which your 
# hardware programmer is attached
#
#AVRDUDE_PORT=

# Include global config file
-include ../Make.config
-include ../general.mk
-include ../build.mk

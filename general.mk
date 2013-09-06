# determine CPU frequency based on media type
CUSTOM_CFLAGS += -DREVISION=$(REVISION)
ifeq ($(MEDIATYPE), tp)
	CUSTOM_CFLAGS += -DF_CPU=8000000UL -DFB_TP
endif
ifeq ($(MEDIATYPE), tprf)
	CUSTOM_CFLAGS += -DF_CPU=10000000UL -DFB_TP -DFB_RF
endif
ifeq ($(MEDIATYPE), rf)
	CUSTOM_CFLAGS += -DF_CPU=10000000UL -DFB_RF
endif
ifeq ($MEDIATYPE), tpuart)
	CUSTOM_CFLAGS += -DF_CPU=8000000UL -DFB_TPUART
endif

CUSTOM_CFLAGS += -DUSE_UART

INC?=-I../include/
# build lib name
LIBS?=-lfb_$(MCU)_$(MEDIATYPE)_$(REVISION)$(DEBUG) -L..

# definitions for avrdude
PROGRAMMER_MCU?=$(MCU)


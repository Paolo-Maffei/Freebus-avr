# determine libraries to link in and CPU frequency, depending on RF
ifneq (,$(findstring FB_RF,$(CUSTOM_CFLAGS)))
	CUSTOM_CFLAGS += -DF_CPU=10000000UL
	ifneq (,$(findstring FB_TP,$(CUSTOM_CFLAGS)))
		LIBS?=-lfbtprf$(DEBUG) -L..
	else
		LIBS?=-lfbrf$(DEBUG) -L..
	endif
else
	ifneq (,$(findstring FB_TP,$(CUSTOM_CFLAGS)))
		CUSTOM_CFLAGS += -DF_CPU=8000000UL
		ifneq (,$(findstring BOARD301,$(CUSTOM_CFLAGS)))
			LIBS?=-lavreib$(DEBUG) -L..
		else
			LIBS?=-lfbtp$(DEBUG) -L..
		endif
	else
		LIBS?=-lavreib$(DEBUG) -L..
	endif
endif

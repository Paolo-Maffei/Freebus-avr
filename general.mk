# determine libraries to link in and CPU frequency, depending on RF
ifneq (,$(findstring DDEBUG_UART,$(CUSTOM_CFLAGS)))
  # Compile with debug
  ifneq (,$(findstring FB_RF,$(CUSTOM_CFLAGS)))
    CUSTOM_CFLAGS += -DF_CPU=10000000UL
    ifneq (,$(findstring FB_TP,$(CUSTOM_CFLAGS)))
        LIBS?=-lfbtprfdebug -L..
    else
        LIBS?=-lfbrfdebug -L..
    endif
  else
    ifneq (,$(findstring FB_TP,$(CUSTOM_CFLAGS)))
      CUSTOM_CFLAGS += -DF_CPU=8000000UL
      ifneq (,$(findstring BOARD301,$(CUSTOM_CFLAGS)))
        LIBS?=-lavreibdebug -L..
      else
        LIBS?=-lfbtpdebug -L..
      endif
    else
      LIBS?=-lavreibdebug -L..
    endif
  endif
else
  # Compile without debug
  ifneq (,$(findstring FB_RF,$(CUSTOM_CFLAGS)))
    CUSTOM_CFLAGS += -DF_CPU=10000000UL
    ifneq (,$(findstring FB_TP,$(CUSTOM_CFLAGS)))
      LIBS?=-lfbtprf -L..
    else
      LIBS?=-lfbrf -L..
    endif
  else
    ifneq (,$(findstring FB_TP,$(CUSTOM_CFLAGS)))
      CUSTOM_CFLAGS += -DF_CPU=8000000UL
      ifneq (,$(findstring BOARD301,$(CUSTOM_CFLAGS)))
        LIBS?=-lavreib -L..
      else
        LIBS?=-lfbtp -L..
      endif
    else
      LIBS?=-lavreib -L..
    endif
  endif
endif
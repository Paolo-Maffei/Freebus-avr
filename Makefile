#DIRECTORIES = app-8-out app-8-in app-2-dimme rf-app-pirr
#DIRECTORIES = app-8-out app-8-in
DIRECTORIES = app-8-out app-8-in app-4-rollo

all:
	@for i in $(DIRECTORIES); do $(MAKE) -C $$i; done

debug:
	@for i in $(DIRECTORIES); do $(MAKE) -C $$i debug; done

doc:
	-doxygen

hex:
	@for i in $(DIRECTORIES); do $(MAKE) -C $$i hex; done
bin:
	@for i in $(DIRECTORIES); do $(MAKE) -C $$i bin; done

debug-hex:
	@for i in $(DIRECTORIES); do $(MAKE) -C $$i debug-hex; done

debug-bin:
	@for i in $(DIRECTORIES); do $(MAKE) -C $$i debug-bin; done

clean:
	@for i in $(DIRECTORIES); do $(MAKE) -C $$i clean; done

distclean:
	@for i in $(DIRECTORIES); do $(MAKE) -C $$i distclean; done

debug-clean:
	@for i in $(DIRECTORIES); do $(MAKE) -C $$i debug-clean; done

debug-distclean:
	@for i in $(DIRECTORIES); do $(MAKE) -C $$i debug-distclean; done

stats:
	@for i in $(DIRECTORIES); do $(MAKE) -C $$i stats; done

avrlib:
	$(MAKE) -C lib
	$(MAKE) -C lib install

avrlib-debug:
	$(MAKE) -C lib debug
	$(MAKE) -C lib debug-install

avrlib-clean:
	$(MAKE) -C lib clean

.PHONY: avrlib-distclean avrlib-clean
avrlib-distclean: avrlib-clean
	$(RM) *.a

avrlib-install:
	$(MAKE) -C lib install

# DO NOT DELETE

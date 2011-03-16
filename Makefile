#DIRECTORIES = app-8-out app-8-in app-2-dimme rf-app-pirr
DIRECTORIES = app-8-out app-8-in

all:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i; done

debug:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i debug; done

doc:
	-doxygen

hex:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i hex; done

debug-hex:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i debug-hex; done

clean:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i clean; done

debug-clean:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i debug-clean; done

stats:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i stats; done

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

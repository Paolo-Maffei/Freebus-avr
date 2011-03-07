#DIRECTORIES = app-8-out app-8-in app-2-dimme rf-app-pirr
DIRECTORIES = app-8-out

all:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i; done

doc:
	-doxygen

hex:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i hex; done

clean:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i clean; done

stats:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i stats; done

avrlib:
	$(MAKE) -C lib
	$(MAKE) -C lib install

avrlib-clean:
	$(MAKE) -C lib clean

avrlib-distclean:
	$(MAKE) -C lib clean
	$(RM) *.a

avrlib-install:
	$(MAKE) -C lib install

# DO NOT DELETE

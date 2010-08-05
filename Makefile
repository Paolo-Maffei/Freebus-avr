#DIRECTORIES = app-8-out app-8-in app-2-dimmer
DIRECTORIES = app-8-out app-8-in

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

# DO NOT DELETE

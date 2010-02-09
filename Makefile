#DIRECTORIES = app-8-out app-8-in app-2-dimmer
DIRECTORIES = app-8-out app-8-in

all:
	-doxygen
	for i in $(DIRECTORIES); do $(MAKE) -C $$i; done

hex:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i hex; done

clean:
	for i in $(DIRECTORIES); do $(MAKE) -C $$i clean; done

avrlib:
	$(MAKE) -C lib
	$(MAKE) -C lib install

# DO NOT DELETE

CC = gcc

KDIR = minimal-linux
KBUILD = build-kernel.sh
KBUILDDIR = $(KDIR)/build

.PHONE : all clean

all:
	cd $(KDIR) && chmod +x $(KBUILD)
	cd $(KDIR) && ./$(KBUILD)

clean:
	$(RM) -r $(KBUILDDIR)

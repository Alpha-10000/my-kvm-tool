CC = gcc

KDIR = minimal-linux
KBUILD = build-kernel.sh
KBUILDDIR = $(KDIR)/build

INITRDDIR = $(KBUILDDIR)/initramfs
INITRD = initramfs.img

.PHONE : all clean

all:
	cd $(KDIR) && chmod +x $(KBUILD) && ./$(KBUILD)
	mkdir -p $(INITRDDIR) && cd $(INITRDDIR) && sudo mkinitcpio -g $(INITRD)

clean:
	$(RM) -r $(KBUILDDIR)

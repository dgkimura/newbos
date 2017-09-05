CC=$(HOME)/opt/cross/bin/i686-elf-gcc
AR=$(HOME)/opt/cross/bin/i686-elf-ar
AS=$(HOME)/opt/cross/bin/i686-elf-as

CFLAGS=-ffreestanding -Wall -Wextra -nostdlib -lgcc -O2
LDFLAGS=

ARCHDIR=kernel/arch/i386

SOURCES=\
kernel/kernel.c \
lib/string.c \

OBJECTS=$(SOURCES:.c=.o)

ARCH_SOURCES=\
$(ARCHDIR)/boot.s \
$(ARCHDIR)/idt.c \
$(ARCHDIR)/idt.s \
$(ARCHDIR)/isr.c \
$(ARCHDIR)/irq.c \
$(ARCHDIR)/irq.s \
$(ARCHDIR)/gdt.c \
$(ARCHDIR)/gdt.s \
$(ARCHDIR)/timer.c \
$(ARCHDIR)/tty.c \
$(ARCHDIR)/setup.c \

ARCH_OBJECTS=$(ARCH_SOURCES:.s=.o)
ARCH_OBJECTS=$(ARCH_SOURCES:.c=.o)

EXECUTABLE=newbos.bin libc.a

all: $(OBJECTS) $(EXECUTABLE) $(ARCH_OBJECTS)

newbos.bin: libc.a $(OBJECTS) $(ARCH_OBJECTS)
	$(CC) $(CFLAGS) -T $(ARCHDIR)/linker.ld -o newbos.bin $(OBJECTS) $(ARCH_OBJECTS)

libc.a: $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ -I include -I kernel/include

.s.o:
	$(AS) -c $< -o $@

clean:
	rm -f libc.a newbos.bin
	rm -f *.o */*.o */*/*.o */*/*/*.o

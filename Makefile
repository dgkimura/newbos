CFLAGS=-ffreestanding -Wall -Wextra -nostdlib -g -O2
LDFLAGS=

ARCHDIR=kernel/arch/i386

SOURCES=\
kernel/kernel.c \
kernel/kmalloc.c \
kernel/printk.c \
kernel/process.c \
lib/stdlib.c \
lib/string.c \

OBJECTS=$(SOURCES:.c=.o)

ARCH_SOURCES=\
$(ARCHDIR)/boot.s \
$(ARCHDIR)/interrupts.c \
$(ARCHDIR)/interrupts.s \
$(ARCHDIR)/gdt.c \
$(ARCHDIR)/gdt.s \
$(ARCHDIR)/io.s \
$(ARCHDIR)/keyboard.c \
$(ARCHDIR)/paging.c \
$(ARCHDIR)/paging.s \
$(ARCHDIR)/timer.c \
$(ARCHDIR)/tty.c \

ARCH_OBJECTS=$(ARCH_SOURCES:.s=.o)
ARCH_OBJECTS=$(ARCH_SOURCES:.c=.o)

EXECUTABLE=newbos.bin libc.a

all: $(OBJECTS) $(EXECUTABLE) $(ARCH_OBJECTS)

newbos.bin: libc.a $(OBJECTS) $(ARCH_OBJECTS)
	$(CC) $(CFLAGS) -T $(ARCHDIR)/linker.ld -o newbos.bin $(OBJECTS) $(ARCH_OBJECTS)

libc.a: $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ -I include -I kernel/include -I $(ARCHDIR)

.s.o:
	$(AS) -c $< -o $@

clean:
	rm -f libc.a newbos.bin
	rm -f *.o */*.o */*/*.o */*/*/*.o

CFLAGS=-ffreestanding -Wall -Wextra -nostdlib -g -O2
LDFLAGS=-T $(ARCHDIR)/linker.ld -melf_i386

ARCHDIR=kernel/arch/i386

SOURCES=\
kernel/kernel.c \
kernel/kmalloc.c \
kernel/printk.c \
kernel/process.c \
kernel/scheduler.c \
lib/stdlib.c \
lib/string.c \
$(ARCHDIR)/gdt.c \
$(ARCHDIR)/interrupts.c \
$(ARCHDIR)/keyboard.c \
$(ARCHDIR)/paging.c \
$(ARCHDIR)/timer.c \
$(ARCHDIR)/tty.c \

OBJECTS=$(SOURCES:.c=.o)

ASSEMBLY_SOURCES=\
$(ARCHDIR)/boot.s \
$(ARCHDIR)/interrupts_assembler.s \
$(ARCHDIR)/gdt_assembler.s \
$(ARCHDIR)/io.s \
$(ARCHDIR)/paging_assembler.s \
$(ARCHDIR)/scheduler_assembler.s \

ASSEMBLY_OBJECTS=$(ASSEMBLY_SOURCES:.s=.o)

EXECUTABLE=newbos.bin libc.a newbos.elf

all: $(OBJECTS) $(EXECUTABLE) $(ASSEMBLY_OBJECTS)

newbos.elf: $(OBJECTS) $(ASSEMBLY_OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(ASSEMBLY_OBJECTS) -o newbos.elf -I include -I kernel/include -I $(ARCHDIR)

newbos.bin: libc.a $(OBJECTS) $(ASSEMBLY_OBJECTS)
	$(CC) $(CFLAGS) -T $(ARCHDIR)/linker.ld -o newbos.bin $(OBJECTS) $(ASSEMBLY_OBJECTS)

libc.a: $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ -I include -I kernel/include -I $(ARCHDIR)

.s.o:
	$(CC) -c $< -o $@

clean:
	rm -f libc.a newbos.bin
	rm -f *.o */*.o */*/*.o */*/*/*.o

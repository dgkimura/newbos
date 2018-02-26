#include <newbos/printk.h>

#include "interrupts.h"
#include "io.h"

unsigned char keyboard_layout[128] =
{
    0,
    27,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    '\b',
    '\t',
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n',
    0,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    0,
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '-',
    0,
    0,
    0,
    '+',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /* All other keys are undefined */
};

void
keyboard_callback(registers_t* regs)
{
    uint8_t scancode;

    // Read from the keyboard data buffer
    scancode = inb(0x60);

    // If the top bit of the byte we read from the keyboard is set, that means
    // the key has just been released.
    if (scancode & 0x80)
    {
        // We can use this one to see if the user has just released the shift,
        // alt, or control keys...
    }
    else
    {
        // Here, a key was just pressed. Please note that if you hold a keydown
        // then you will get repeated key press interrupts.

        // Just to show how this works we simply translate the keyboard
        // scancode into an ASCII value, and then display it on the screen. You
        // can get creative and use some flags to see if shift is pressed and
        // use a different layout, or you could add another 128 entries to the
        // above layout to correspond to 'shift' being held. If shift is held
        // using the larger lookup table, you would add 128 to the scancode
        //when you look for it.
        printk("%s", keyboard_layout[scancode]);
    }
}

void
keyboard_init()
{
    register_irq_handler(IRQ1, keyboard_callback);
}

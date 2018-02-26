#include <stdarg.h>
#include <stdint.h>

#include <newbos/printk.h>
#include <newbos/tty.h>

void
printk(
    char *fmt,
    ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *p;
    uint32_t uival;
    char *sval;

    for (p = fmt; *p != '\0'; ++p)
    {
        if (*p != '%') {
            tty_write_byte(*p);
            continue;
        }

        switch (*++p) {
            case 'X':
                uival = va_arg(ap, uint32_t);
                tty_write_hex(uival);
                break;
            case 's':
                sval = va_arg(ap, char*);
                tty_write_string(sval);
                break;
            case '%':
                tty_write_byte('%');
                break;
        }
    }

    va_end(ap);
}

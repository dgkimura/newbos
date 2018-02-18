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
            monitor_put(*p);
            continue;
        }

        switch (*++p) {
            case 'X':
                uival = va_arg(ap, uint32_t);
                monitor_write_hex(uival);
                break;
            case 's':
                sval = va_arg(ap, char*);
                for(; *sval; ++sval) {
                    monitor_put(*sval);
                }
                break;
            case '%':
                monitor_put('%');
                break;
        }
    }

    va_end(ap);
}

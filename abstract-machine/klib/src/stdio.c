#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

static int _itoa(long value, char *str, int base, int is_signed) {
  if (base < 2 || base > 36) {
    str[0] = '\0';
    return 0;
  }

  char *ptr = str;
  char *head = str;
  
  int is_negative = 0;
  unsigned long uvalue;
  if (is_signed && value < 0 && base == 10) {
    is_negative = 1;
    uvalue = (unsigned long)(-value);
  } else {
    uvalue = (unsigned long)value;
  }

  do {
    int digit = uvalue % base;
    *ptr++ = (digit < 10) ? '0' + digit : 'a' + digit - 10;
    uvalue /= base;
  } while (uvalue > 0);

  if (is_negative) {
    *ptr++ = '-';
  }
  *ptr = '\0';

  int len = ptr - head;
  
  ptr--;
  char temp;
  while (head < ptr) {
    temp = *head;
    *head++ = *ptr;
    *ptr-- = temp;
  }

  return len;
}

static int _ftoa(double value, char *str, int precision) {
  if (precision < 0)
    precision = 6;

  int is_negative = 0;
  if (value < 0) {
    is_negative = 1;
    value = -value;
  }

  char *ptr = str;

  long int_part = (long)value;
  double frac_part = value - int_part;

  if (is_negative) {
    *ptr++ = '-';
  }

  int int_len = _itoa(int_part, ptr, 10, 0);
  ptr += int_len;

  if (precision > 0) {
    *ptr++ = '.';

    for (int i = 0; i < precision; i++) {
      frac_part *= 10;
      int digit = (int)frac_part;
      *ptr++ = '0' + digit;
      frac_part -= digit;
    }
  }

  *ptr = '\0';
  return ptr - str;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *dst = out;
  char buffer[32];

  while (*fmt) {
    if (*fmt != '%') {
      *dst++ = *fmt++;
      continue;
    }

    fmt++;

    /* todo Flag characters */

    int width = 0;
    if (*fmt >= '0' && *fmt <= '9') {
      while (*fmt >= '0' && *fmt <= '9') {
        width = width * 10 + (*fmt - '0');
        fmt++;
      }
    }

    int precision = -1;
    if (*fmt == '.') {
      fmt++;
      precision = 0;
      while (*fmt >= '0' && *fmt <= '9') {
        precision = precision * 10 + (*fmt - '0');
        fmt++;
      }
    }

    switch (*fmt) {
    case 'd':
    case 'i': {
      int ival = va_arg(ap, int);
      int len = _itoa(ival, buffer, 10, 1);

      if (width > len) {
        for (int i = 0; i < width - len; i++) {
          *dst++ = ' ';
        }
      }

      memcpy(dst, buffer, len);
      dst += len;
      break;
    }

    case 'u': {
      unsigned int uval = va_arg(ap, unsigned int);
      int len = _itoa(uval, buffer, 10, 0);

      if (width > len) {
        for (int i = 0; i < width - len; i++) {
          *dst++ = ' ';
        }
      }

      memcpy(dst, buffer, len);
      dst += len;
      break;
    }

    case 'x':
    case 'X': {
      unsigned int uval = va_arg(ap, unsigned int);
      int len = _itoa(uval, buffer, 16, 0);

      if (width > len) {
        for (int i = 0; i < width - len; i++) {
          *dst++ = ' ';
        }
      }

      memcpy(dst, buffer, len);
      dst += len;
      break;
    }

    case 'f': {
      double dval = va_arg(ap, double);
      int len = _ftoa(dval, buffer, precision);

      if (width > len) {
        for (int i = 0; i < width - len; i++) {
          *dst++ = ' ';
        }
      }

      memcpy(dst, buffer, len);
      dst += len;
      break;
    }

    case 'c': {
      char c = (char)va_arg(ap, int);

      if (width > 1) {
        for (int i = 0; i < width - 1; i++) {
          *dst++ = ' ';
        }
      }

      *dst++ = c;
      break;
    }

    case 's': {
      char *s = va_arg(ap, char *);
      if (!s)
        s = "(null)";
      size_t len = strlen(s);

      if (precision >= 0 && len > precision) {
        len = precision;
      }

      if (width > len) {
        for (int i = 0; i < width - len; i++) {
          *dst++ = ' ';
        }
      }

      memcpy(dst, s, len);
      dst += len;
      break;
    }

    case 'p': {
      void *p = va_arg(ap, void *);
      unsigned long addr = (unsigned long)p;
      buffer[0] = '0';
      buffer[1] = 'x';
      int len = _itoa(addr, buffer + 2, 16, 0) + 2;

      if (width > len) {
        for (int i = 0; i < width - len; i++) {
          *dst++ = ' ';
        }
      }

      memcpy(dst, buffer, len);
      dst += len;
      break;
    }

    case '%': {
      *dst++ = '%';
      break;
    }

    default: {
      *dst++ = '%';
      *dst++ = *fmt;
      break;
    }
    }

    fmt++;
  }

  *dst = '\0';
  return dst - out;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif

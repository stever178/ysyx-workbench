#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

const size_t word_size = sizeof(size_t);
const size_t align_mask = word_size - 1;

size_t strlen(const char *s) {
  size_t ret = 0;
  while (*s != '\0') {
    ret++;
    s++;
  }
  return ret;
}

/* The strings src and dst may not overlap. */
char *strcpy(char *dst, const char *src) {
  char *ret = dst;
  for (; *src != '\0'; dst++, src++) {
    *dst = *src;
  }
  *dst = *src;
  return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *ret = dst;
  for (; n > 0; n--, dst++, src++) {
    if (*src == '\0') {
      break;
    }
    *dst = *src;
  }
  while (n > 0) {
    *dst = '\0';
    n--;
    dst++;
  }
  return ret;
}

/* The strings src and dst may not overlap. */
char *strcat(char *dst, const char *src) {
  char *ret = dst;
  while (*dst != '\0') {
    dst++;
  }
  strcpy(dst, src);
  return ret;
}

int strcmp(const char *s1, const char *s2) {
  for (; *s1 != '\0' && *s2 != '\0'; s1++, s2++) {
    if (*s1 != *s2) {
      return *s1 - *s2;
    }
  }
  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (; n > 0 && *s1 != '\0' && *s2 != '\0'; n--, s1++, s2++) {
    if (*s1 != *s2) {
      return *s1 - *s2;
    }
  }
  return *s1 - *s2;
}

void *memset(void *s, int c, size_t n) {
  void *ret = s;
  
  const char data = (char)c;
  size_t word = 0;
  for (size_t i = 0; i < word_size; i++) {
    word = (word << 8) | data;
  }

  char *dst = s;
  for (; n > 0 && ((uintptr_t)dst & align_mask); n--) {
    *dst++ = data;
  }

  size_t *word_ptr = (size_t *)dst;
  for (; n >= word_size; n -= word_size) {
    *word_ptr++ = word;
  }

  dst = (char *)word_ptr;
  for (; n > 0; n--) {
    *dst++ = data;
  }

  return ret;
}

/* The memory areas may overlap.
DESCRIPTION
   The memmove() function copies n bytes from memory area src to memory area
dest. The memory areas may overlap: copying takes place as though the bytes in
src are first copied into a temporary array that does not overlap src or dest,
and the bytes are then copied from the temporary array to dest.
*/
void *memmove(void *dst, const void *src, size_t n) {
  void *ret = dst;

  char *dst_ptr = dst;
  const char *src_ptr = src;

  if (dst < src) {
    for (; n > 0; n--, dst++, src++) {
      *dst_ptr = *src_ptr;
    }
  } else if (dst > src) {
    for (size_t i = n - 1; i >= 0; i--) {
      dst_ptr[i] = src_ptr[i];
    }
  }

  return ret;
}

/* The memory areas must not overlap. */
void *memcpy(void *out, const void *in, size_t n) {
  void *ret = out;

  char *dst = out;
  const char *src = in;
  for (; n > 0 && ((uintptr_t)dst & align_mask); n--) {
    *dst++ = *src++;
  }

  size_t *dst_word_ptr = (size_t *)dst;
  const size_t *src_word_ptr = (size_t *)src;
  for (; n >= word_size; n -= word_size) {
    *dst_word_ptr++ = *src_word_ptr++;
  }

  dst = (char *)dst_word_ptr;
  src = (char *)src_word_ptr;
  for (; n > 0; n--, dst++, src++) {
    *dst = *src;
  }

  return ret;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const char *dst = s1;
  const char *src = s2;
  for (; n > 0 && ((uintptr_t)dst & align_mask); n--, dst++, src++) {
    if (*dst != *src) {
      return *dst - *src;
    }
  }

  const size_t *dst_word_ptr = (size_t *)dst;
  const size_t *src_word_ptr = (size_t *)src;
  for (; n >= word_size; n -= word_size, dst_word_ptr++, src_word_ptr++) {
    if (*dst_word_ptr != *src_word_ptr) {
      return *dst_word_ptr - *src_word_ptr;
    }
  }

  dst = (char *)dst_word_ptr;
  src = (char *)src_word_ptr;
  for (; n > 0; n--, dst++, src++) {
    if (*dst != *src) {
      return *dst - *src;
    }
  }

  return 0;
}

#endif

#include "fio.h"

void *memcpy(void *dest, const void *src, size_t n);

int strcmp(const char *a, const char *b) __attribute__ ((naked));
int strcmp(const char *a, const char *b);

int strncmp(const char *a, const char *b, size_t n);

size_t strlen(const char *s) __attribute__ ((naked));
size_t strlen(const char *s);

void itoa(int n, char *buffer);

int str_to_int(char *str);
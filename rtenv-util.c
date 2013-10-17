#include "fio.h"
#include "rtenv-util.h"

void *memcpy(void *dest, const void *src, size_t n);

int strcmp(const char *a, const char *b) __attribute__ ((naked));
int strcmp(const char *a, const char *b)
{
        asm(
        "strcmp_lop:                \n"
        "   ldrb    r2, [r0],#1     \n"
        "   ldrb    r3, [r1],#1     \n"
        "   cmp     r2, #1          \n"
        "   it      hi              \n"
        "   cmphi   r2, r3          \n"
        "   beq     strcmp_lop      \n"
                "        sub     r0, r2, r3          \n"
        "   bx      lr              \n"
                :::
        );
}

int strncmp(const char *a, const char *b, size_t n)
{
        size_t i;

        for (i = 0; i < n; i++)
                if (a[i] != b[i])
                        return a[i] - b[i];

        return 0;
}

size_t strlen(const char *s) __attribute__ ((naked));
size_t strlen(const char *s)
{
        asm(
                "        sub  r3, r0, #1                        \n"
        "strlen_loop:               \n"
                "        ldrb r2, [r3, #1]!                \n"
                "        cmp  r2, #0                                \n"
        "   bne  strlen_loop        \n"
                "        sub  r0, r3, r0                        \n"
                "        bx   lr                                        \n"
                :::
        );
}


void itoa(int n, char *buffer)
{
        if (n == 0)
                *(buffer++) = '0';
        else {
                int f = 10000;

                if (n < 0) {
                        *(buffer++) = '-';
                        n = -n;
                }

                while (f != 0) {
                        int i = n / f;
                        if (i != 0) {
                                *(buffer++) = '0'+(i%10);;
                        }
                        f/=10;
                }
        }
        *buffer = '\0';
}

int str_to_int(char *str)
{
        int i=0,tmp=0;
        while(str[i]!='\0')
        {
                /*using ascii code to do this comparison*/
                if (str[i]>='0'&&str[i]<='9') tmp = tmp*10 + (str[i]-'0');
                else return -1;
                i++;
        }
        return tmp;
}
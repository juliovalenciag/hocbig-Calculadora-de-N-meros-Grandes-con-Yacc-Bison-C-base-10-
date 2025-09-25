#ifndef BIG_CALC_H
#define BIG_CALC_H

#include <stddef.h>
#include <stdbool.h>

#define BIG_BASE 10000
#define BIG_BLEN 4

typedef struct {
    int *c;  // bloques base 10^4, little-endian
    int n;   // bloques en uso
    int cap; // capacidad
    int sign; // +1 o -1 (cero => +1)
} Big;

typedef Big* BigAP;

/* construcción / destrucción */
Big *big_new(int cap);
Big *big_from_string(const char *txt);
char *big_to_string(const Big *a);
void big_free(Big *a);

/* utilidades */
Big *big_clone(const Big *a);
void big_trim(Big *a);
int  big_cmp_abs(const Big *a, const Big *b);
int  big_cmp(const Big *a, const Big *b);
bool big_is_zero(const Big *a);

/* aritmética con signo */
Big *big_add(const Big *a, const Big *b);
Big *big_sub(const Big *a, const Big *b);
Big *big_mul(const Big *a, const Big *b);
void big_divmod(const Big *a, const Big *b, Big **q_out, Big **r_out);

/* potencia (exponente ULL >=0) */
Big *big_pow_small(const Big *base, unsigned long long e);

/* I/O */
void imprimeBig(const Big *a);

#endif

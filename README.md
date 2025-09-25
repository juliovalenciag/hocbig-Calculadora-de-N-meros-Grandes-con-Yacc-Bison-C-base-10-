# hocbig — Calculadora de Números Grandes (C + Yacc/Bison)

Calculadora de **enteros grandes** (mucho más allá de 64 bits) con un
**parser** hecho en Yacc/Bison y un **núcleo aritmético** en C que usa base
**10 000** (4 dígitos por bloque). Imprime los resultados en **decimal normal**
sin espacios ni ceros perdidos.

## ✨ Características

- Enteros con **signo** (positivos y negativos).
- Operaciones: `+`, `-`, `*`, `/`, `%`, `^` y **paréntesis**.
- Precedencia estándar:
  - `^` (asociativa a la **derecha**)
  - unarios `+ -`
  - `* / %`
  - `+ -`
- Núcleo en base **10 000** (más rápido que base 10).
- Salida en **decimal continuo** (sin ceros truncados).
- Implementación **portable** (sin `strdup` POSIX; se incluye `xstrdup`).

> Nota: por diseño, `^` acepta exponente entero **no negativo** que quepa en
> `unsigned long long`. En la sección _Roadmap_ hay ideas para exponente “big”.

---

## 📁 Estructura

.
├─ big_calc.h # API y estructura Big (signo, bloques base 10^4)
├─ big_calc.c # Implementaciones: suma, resta, mul, div, mod, pow, I/O
├─ big_calc.y # Gramática y scanner mínimo (sin flex), usa xstrdup()
├─ Makefile # Compila en macOS/Linux con bison o yacc/byacc
└─ README.md

yaml
Copiar código

---

## 🛠️ Requisitos

- Compilador C11 (clang o gcc).
- **Bison** (recomendado) o `yacc`/`byacc`.

En macOS con Homebrew:

```bash
brew install bison
export PATH="/opt/homebrew/opt/bison/bin:$PATH"      # Apple Silicon
# ó: export PATH="/usr/local/opt/bison/bin:$PATH"    # Intel
```

🚀 Compilación
```bash
make        # genera ./hocbig
```
Si quieres limpiar:
```bash

make clean
```
> El Makefile está configurado para no renombrar y.tab.c. Si ves en tu
> salida un mv y.tab.c big_calc.c, estás usando un Makefile viejo.

▶️ Uso básico
Ejecuta y escribe una expresión por línea; Enter evalúa:

```bash
./hocbig
123456789012345678901234567890 + 123000000000000000000000000000001
(99999999999999999999 + 1) * 2


```
Salidas esperadas (línea a línea):
```bash
diff
Copiar código
123123456789012345678901234567891
200000000000000000000
```

Convención de resto: mismo signo del dividendo (como en C).

🧩 API (para integrar en otros proyectos)
Declarada en big_calc.h:

- Tipos
  - typedef struct Big
  - int *c; /* bloques base 10^4, little-endian */
  - int n, cap, sign; /* +1/-1; cero => +1 */
- Construcción/I/O
  - Big *big_new(int cap);
  - Big *big_from_string(const char *txt); // +/- opcional
  - char *big_to_string(const Big *a); // malloc internamente
  - void imprimeBig(const Big *a);
  - void big_free(Big *a);
- Utilidades
  - Big *big_clone(const Big *a);
  - void big_trim(Big *a);
  - int big_cmp_abs(const Big *a, const Big *b);
  - int big_cmp(const Big *a, const Big *b);

- Aritmética
  - Big *big_add(const Big *a, const Big *b);
  - Big *big_sub(const Big *a, const Big *b);
  - Big *big_mul(const Big *a, const Big *b);
  - void big_divmod(const Big *a, const Big *b, Big **q, Big **r);
  - Big *big_pow_small(const Big *base, unsigned long long e);

> Todas las funciones que retornan Big* devuelven memoria que debes liberar
> con big_free.

🏗️ Notas de implementación
- Representación en base 10 000 (BIG_BASE = 10000) con little-endian (el bloque menos significativo está en c[0]).

- big_to_string imprime el bloque más significativo sin padding y el resto con relleno a 4 dígitos, garantizando decimales correctos.

- División larga en base 10⁴ con estimación por los dos bloques MSB; evita normalización por escala para mantener el código didáctico y portable.

- El scanner integrado (en big_calc.y) evita dependencias de flex.

- Reemplazo portátil de strdup → xstrdup (para C11 puro).

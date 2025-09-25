# Calculadora de números grandes
# Uso: make | make run | make clean

CC      := cc
CFLAGS  := -O2 -Wall -Wextra -std=c11

# Elige una:
YACC    := bison -y -d     # si tienes bison
# YACC := yacc -d          # o byacc/yacc (comenta la otra)

all: hocbig

y.tab.c y.tab.h: big_calc.y big_calc.h
	$(YACC) big_calc.y

y.tab.o: y.tab.c big_calc.h
	$(CC) $(CFLAGS) -c y.tab.c

big_calc.o: big_calc.c big_calc.h
	$(CC) $(CFLAGS) -c big_calc.c

hocbig: y.tab.o big_calc.o
	$(CC) $(CFLAGS) -o hocbig y.tab.o big_calc.o

run: hocbig
	./hocbig

clean:
	rm -f hocbig y.tab.c y.tab.h y.tab.o big_calc.o
.PHONY: all run clean

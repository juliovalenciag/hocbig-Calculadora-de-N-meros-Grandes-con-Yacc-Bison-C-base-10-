~~~
bison -y -d big_calc.y
cc -O2 -Wall -Wextra -std=c11 -c big_calc.c
cc -O2 -Wall -Wextra -std=c11 -o hocbig y.tab.c big_calc.o
~~~
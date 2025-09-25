%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "big_calc.h"

int yylex(void);
int yyerror(const char *s){ (void)s; fprintf(stderr,"syntax error\n"); return 0; }
%}

%union {
  BigAP val;
  char *s;
}

%token <s> BIGINT
%type  <val> expr

%right '^'
%right UMINUS UPLUS
%left  '*' '/' '%'
%left  '+' '-'

%%
input  : /* empty */
       | input line
       ;

line   : '\n'
       | expr '\n'     { imprimeBig($1); big_free($1); }
       ;

expr   : BIGINT        { $$ = big_from_string($1); free($1); }
       | '(' expr ')'  { $$ = $2; }

       | expr '+' expr { Big *t = big_add($1,$3); big_free($1); big_free($3); $$ = t; }
       | expr '-' expr { Big *t = big_sub($1,$3); big_free($1); big_free($3); $$ = t; }
       | expr '*' expr { Big *t = big_mul($1,$3); big_free($1); big_free($3); $$ = t; }
       | expr '/' expr { Big *q,*r; big_divmod($1,$3,&q,&r); big_free($1); big_free($3); big_free(r); $$ = q; }
       | expr '%' expr { Big *q,*r; big_divmod($1,$3,&q,&r); big_free($1); big_free($3); big_free(q); $$ = r; }
       | expr '^' expr {
            char *exp_str = big_to_string($3); const char *p = exp_str;
            if(*p=='-'){ free(exp_str); big_free($1); big_free($3); yyerror("negative exponent"); YYERROR; }
            unsigned long long e=0;
            while(*p){
              if(!isdigit((unsigned char)*p)){ free(exp_str); big_free($1); big_free($3); yyerror("invalid exponent"); YYERROR; }
              unsigned d=(unsigned)(*p-'0');
              if(e>(~0ULL/10ULL) || (e==~0ULL/10ULL && d>(~0ULL%10ULL))){ free(exp_str); big_free($1); big_free($3); yyerror("exponent too large"); YYERROR; }
              e = e*10ULL + d; p++;
            }
            free(exp_str);
            Big *t = big_pow_small($1,e);
            big_free($1); big_free($3); $$ = t;
       }

       | '-' expr %prec UMINUS { Big *z=big_new(1); Big *t=big_sub(z,$2); big_free(z); big_free($2); $$=t; }
       | '+' expr %prec UPLUS  { $$ = $2; }
       ;
%%

/* Scanner mínimo (sin flex) */
int yylex(void){
   int c;
   do { c=getchar(); } while(c==' '||c=='\t'||c=='\r');

   if(c==EOF) return 0;
   if(c=='\n' || c=='+' || c=='-' || c=='*' || c=='/' || c=='%' || c=='^' || c=='(' || c==')') return c;

   if(isdigit(c)){
      char buf[1<<15]; int k=0; buf[k++]=(char)c;
      while( (c=getchar())!=EOF && isdigit(c) ){ if(k<(int)sizeof(buf)-1) buf[k++]=(char)c; }
      buf[k]='\0'; if(c!=EOF) ungetc(c,stdin);
      yylval.s = strdup(buf); return BIGINT;
   }

   return c;
}

int main(void){ return yyparse(); }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "big_calc.h"

#define BASE BIG_BASE
#define BLEN BIG_BLEN

static void *xmalloc(size_t n){ void *p = malloc(n); if(!p){perror("malloc"); exit(1);} return p; }
static void *xrealloc(void *p, size_t n){ void *q = realloc(p,n); if(!q){perror("realloc"); exit(1);} return q; }

Big *big_new(int cap){
    if(cap < 1) cap = 1;
    Big *a = (Big*)xmalloc(sizeof(Big));
    a->c = (int*)xmalloc(sizeof(int)*cap);
    a->n = 1; a->cap = cap; a->sign = +1;
    for(int i=0;i<cap;i++) a->c[i]=0;
    return a;
}

void big_free(Big *a){ if(!a) return; free(a->c); free(a); }

Big *big_clone(const Big *a){
    Big *b = big_new(a->n);
    b->n=a->n; b->sign=a->sign;
    for(int i=0;i<a->n;i++) b->c[i]=a->c[i];
    return b;
}

static Big *big_ensure(Big *a, int need){
    if(a->cap >= need) return a;
    int newcap = need + (need>>1) + 4;
    a->c = (int*)xrealloc(a->c, sizeof(int)*newcap);
    for(int i=a->cap;i<newcap;i++) a->c[i]=0;
    a->cap = newcap;
    return a;
}

void big_trim(Big *a){
    while(a->n>1 && a->c[a->n-1]==0) a->n--;
    if(a->n==1 && a->c[0]==0) a->sign = +1;
}

bool big_is_zero(const Big *a){ return (a->n==1 && a->c[0]==0); }

Big *big_from_string(const char *txt){
    while(isspace((unsigned char)*txt)) txt++;
    int sign = +1;
    if(*txt=='+'||*txt=='-'){ if(*txt=='-') sign=-1; txt++; }
    while(*txt=='0') txt++;
    if(*txt=='\0' || *txt=='\n'){ Big *z=big_new(1); return z; }

    const char *p=txt; int len=0;
    while(p[len] && p[len]!='\n' && isdigit((unsigned char)p[len])) len++;

    int nb = (len + BLEN - 1)/BLEN;
    Big *a = big_new(nb); a->n=nb; a->sign=sign;

    int idx=0;
    for(int i=len;i>0;i-=BLEN){
        int s=i-BLEN; if(s<0) s=0;
        int v=0;
        for(int k=s;k<i;k++) v = v*10 + (txt[k]-'0');
        a->c[idx++]=v;
    }
    big_trim(a);
    return a;
}

char *big_to_string(const Big *a){
    int alloc = a->n*BLEN + 2;
    char *s = (char*)xmalloc(alloc);
    char *ptr = s;
    if(a->sign<0 && !big_is_zero(a)) *ptr++='-';
    sprintf(ptr, "%d", a->c[a->n-1]); ptr += strlen(ptr);
    for(int i=a->n-2;i>=0;--i){ sprintf(ptr, "%0*d", BLEN, a->c[i]); ptr += BLEN; }
    *ptr='\0'; return s;
}

void imprimeBig(const Big *a){ char *s=big_to_string(a); puts(s); free(s); }

int big_cmp_abs(const Big *a, const Big *b){
    if(a->n!=b->n) return (a->n>b->n)?1:-1;
    for(int i=a->n-1;i>=0;--i) if(a->c[i]!=b->c[i]) return (a->c[i]>b->c[i])?1:-1;
    return 0;
}
int big_cmp(const Big *a, const Big *b){
    if(a->sign!=b->sign) return (a->sign>b->sign)?1:-1;
    return a->sign * big_cmp_abs(a,b);
}

static Big *add_abs(const Big *a, const Big *b){
    int n=(a->n>b->n?a->n:b->n);
    Big *c=big_new(n+1);
    long long carry=0;
    for(int i=0;i<n;i++){
        long long ai=(i<a->n?a->c[i]:0), bi=(i<b->n?b->c[i]:0);
        long long s=ai+bi+carry;
        c->c[i]=(int)(s%BASE); carry=s/BASE;
    }
    c->n = carry? n+1 : n; if(carry) c->c[n]=(int)carry;
    return c;
}
static Big *sub_abs_ge(const Big *a, const Big *b){
    Big *c=big_new(a->n);
    long long borrow=0;
    for(int i=0;i<a->n;i++){
        long long ai=a->c[i], bi=(i<b->n?b->c[i]:0);
        long long s=ai-bi-borrow;
        if(s<0){s+=BASE; borrow=1;} else borrow=0;
        c->c[i]=(int)s;
    }
    c->n=a->n; big_trim(c); return c;
}

Big *big_add(const Big *a, const Big *b){
    if(a->sign==b->sign){ Big *c=add_abs(a,b); c->sign=a->sign; return c; }
    int cmp=big_cmp_abs(a,b);
    if(cmp==0){ Big *z=big_new(1); return z; }
    if(cmp>0){ Big *c=sub_abs_ge(a,b); c->sign=a->sign; return c; }
    Big *c=sub_abs_ge(b,a); c->sign=b->sign; return c;
}
Big *big_sub(const Big *a, const Big *b){ Big t=*b; t.sign=-t.sign; return big_add(a,&t); }

Big *big_mul(const Big *a, const Big *b){
    if(big_is_zero(a)||big_is_zero(b)){ Big *z=big_new(1); return z; }
    Big *c=big_new(a->n+b->n); c->n=a->n+b->n; c->sign=a->sign*b->sign;
    long long carry,cur;
    for(int i=0;i<a->n;i++){
        carry=0;
        long long ai=a->c[i];
        for(int j=0;j<b->n;j++){
            cur=c->c[i+j]+ai*(long long)b->c[j]+carry;
            c->c[i+j]=(int)(cur%BASE);
            carry=cur/BASE;
        }
        c->c[i+b->n]+= (int)carry;
    }
    big_trim(c); return c;
}

/* división absoluta (long division base BASE) */
static void divmod_abs(const Big *a, const Big *b, Big **q_out, Big **r_out){
    if(big_cmp_abs(a,b)<0){ Big *q=big_new(1); Big *r=big_clone(a); r->sign=+1; *q_out=q; *r_out=r; return; }
    if(b->n==1){
        int d=b->c[0]; Big *q=big_new(a->n); long long rem=0;
        for(int i=a->n-1;i>=0;--i){ long long cur=a->c[i]+rem*BASE; q->c[i]=(int)(cur/d); rem=cur%d; }
        q->n=a->n; big_trim(q); Big *r=big_new(1); r->c[0]=(int)rem; *q_out=q; *r_out=r; return;
    }
    Big *q=big_new(a->n - b->n + 1);
    Big *r=big_clone(a); r->sign=+1;
    Big *db=big_clone(b); db->sign=+1;

    int n=b->n, m=a->n-n;
    long long d1=db->c[n-1], d2=(n>=2)?db->c[n-2]:0;
    big_ensure(r, r->n+1); if(r->n==a->n){ r->c[r->n]=0; r->n++; }

    for(int k=m;k>=0;--k){
        long long r2 = (long long)r->c[k+n]*BASE + r->c[k+n-1];
        long long qhat = r2 / d1; if(qhat>=BASE) qhat=BASE-1;
        while(1){
            long long left = qhat*d2;
            long long right = ((r2 - qhat*d1)*BASE) + ((k+n-2>=0)? r->c[k+n-2]:0);
            if(qhat==0 || left<=right) break; qhat--;
        }
        if(qhat>0){
            long long carry=0, borrow=0;
            for(int j=0;j<n;j++){
                long long prod = qhat*(long long)db->c[j] + carry;
                carry = prod/BASE; prod%=BASE;
                long long idx=k+j;
                long long val=(long long)r->c[idx] - prod - borrow;
                if(val<0){ val+=BASE; borrow=1; } else borrow=0;
                r->c[idx]=(int)val;
            }
            long long idxn=k+n;
            long long valn=(long long)r->c[idxn] - carry - borrow;
            if(valn<0){
                valn+=BASE; r->c[idxn]=(int)valn;
                int car=0; for(int j=0;j<n;j++){
                    long long idx=k+j;
                    long long s=r->c[idx]+db->c[j]+car;
                    if(s>=BASE){ r->c[idx]=(int)(s-BASE); car=1; } else { r->c[idx]=(int)s; car=0; }
                }
                r->c[idxn]+=car; qhat--;
            } else r->c[idxn]=(int)valn;
        }
        q->c[k]=(int)qhat;
    }
    q->n=m+1; big_trim(q);
    while(r->n>1 && r->c[r->n-1]==0) r->n--;
    r->sign=+1;

    big_free(db); *q_out=q; *r_out=r;
}

void big_divmod(const Big *a, const Big *b, Big **q_out, Big **r_out){
    if(big_is_zero(b)){ fprintf(stderr,"division by zero\n"); Big *z=big_new(1), *r=big_new(1); *q_out=z; *r_out=r; return; }
    Big *A=big_clone(a); A->sign=+1;
    Big *B=big_clone(b); B->sign=+1;
    Big *q,*r; divmod_abs(A,B,&q,&r);
    q->sign = a->sign * b->sign; if(big_is_zero(q)) q->sign=+1;
    r->sign = a->sign;           if(big_is_zero(r)) r->sign=+1;
    big_free(A); big_free(B);
    *q_out=q; *r_out=r;
}

Big *big_pow_small(const Big *base, unsigned long long e){
    Big *res=big_new(1); res->c[0]=1; res->n=1; res->sign=+1;
    Big *b=big_clone(base);
    int final_sign = (b->sign<0 && (e&1ULL))? -1 : +1;
    b->sign=+1;
    while(e){
        if(e&1ULL){ Big *t=big_mul(res,b); big_free(res); res=t; }
        e>>=1ULL;
        if(e){ Big *t=big_mul(b,b); big_free(b); b=t; }
    }
    big_free(b);
    if(!big_is_zero(res)) res->sign=final_sign;
    return res;
}

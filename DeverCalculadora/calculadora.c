#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "calculadora.h"

#define MAXTOK 512
#define MAXEXPR 1024

static int is_number(const char *t) {
    if (!t || *t == '\0') return 0;

    if (*t == '+' || *t == '-') t++;

    int dot = 0, digit = 0;
    while (*t) {
        if (isdigit(*t)) digit = 1;
        else if (*t == '.') {
            if (dot) return 0;
            dot = 1;
        } else return 0;
        t++;
    }

    return digit;
}

static int is_unary(const char *t) {
    return (
        strcmp(t, "sen") == 0 ||
        strcmp(t, "cos") == 0 ||
        strcmp(t, "tg")  == 0 ||
        strcmp(t, "log") == 0 ||
        strcmp(t, "raiz") == 0
    );
}

static int is_operator(const char *t) {
    return strlen(t) == 1 && strchr("+-*/%^", t[0]);
}

static int prec(char op) {
    if (op == '^') return 4;
    if (op == '*' || op == '/' || op == '%') return 3;
    if (op == '+' || op == '-') return 2;
    return 1;
}

static int is_left_assoc(char op) {
    return op != '^';
}

typedef struct {
    char s[MAXEXPR];
    int p;
} ExprItem;

static void mk_bin(const ExprItem *A, const ExprItem *B, char op, ExprItem *R) {
    int po = prec(op);

    int lp = (A->p < po);
    int rp = (B->p < po) || (B->p == po && !is_left_assoc(op));

    char temp[MAXEXPR];
    temp[0] = '\0';

    if (lp) sprintf(temp + strlen(temp), "(%s)", A->s);
    else strcat(temp, A->s);

    int L = strlen(temp);
    temp[L] = op;
    temp[L+1] = '\0';

    if (rp) sprintf(temp + strlen(temp), "(%s)", B->s);
    else strcat(temp, B->s);

    strcpy(R->s, temp);
    R->p = po;
}

static void mk_uni(const char *func, const ExprItem *A, ExprItem *R) {
    sprintf(R->s, "%s(%s)", func, A->s);
    R->p = 5;
}

char * getFormaInFixa(char *StrPosfixa) {
    static char out[MAXEXPR];
    out[0] = '\0';

    if (!StrPosfixa) return NULL;

    ExprItem stack[MAXTOK];
    int sp = 0;

    char tok[MAXTOK];
    const char *p = StrPosfixa;

    while (*p) {
        while (*p && isspace(*p)) p++;
        if (!*p) break;

        int i = 0;
        while (*p && !isspace(*p) && i < MAXTOK - 1)
            tok[i++] = *p++;
        tok[i] = '\0';

        if (is_number(tok)) {
            ExprItem E;
            strcpy(E.s, tok);
            E.p = 6;
            stack[sp++] = E;
        }
        else if (is_unary(tok)) {
            if (sp < 1) return NULL;
            ExprItem A = stack[--sp];
            ExprItem R;
            mk_uni(tok, &A, &R);
            stack[sp++] = R;
        }
        else if (is_operator(tok)) {
            if (sp < 2) return NULL;
            ExprItem B = stack[--sp];
            ExprItem A = stack[--sp];
            ExprItem R;
            mk_bin(&A, &B, tok[0], &R);
            stack[sp++] = R;
        }
        else return NULL;
    }

    if (sp != 1) return NULL;

    strcpy(out, stack[0].s);
    return out;
}

float getValorPosFixa(char *StrPosfixa) {
    if (!StrPosfixa) return NAN;

    float st[MAXTOK];
    int sp = 0;

    char tok[MAXTOK];
    const char *p = StrPosfixa;

    while (*p) {
        while (*p && isspace(*p)) p++;
        if (!*p) break;

        int i = 0;
        while (*p && !isspace(*p) && i < MAXTOK - 1)
            tok[i++] = *p++;
        tok[i] = '\0';

        if (is_number(tok)) {
            st[sp++] = atof(tok);
        }
        else if (is_unary(tok)) {
            if (sp < 1) return NAN;
            float a = st[--sp];
            float r;

            if (strcmp(tok, "sen") == 0) r = sin(a * M_PI / 180);
            else if (strcmp(tok, "cos") == 0) r = cos(a * M_PI / 180);
            else if (strcmp(tok, "tg") == 0)  r = tan(a * M_PI / 180);
            else if (strcmp(tok, "log") == 0) {
                if (a <= 0) return NAN;
                r = log10(a);
            }
            else if (strcmp(tok, "raiz") == 0) {
                if (a < 0) return NAN;
                r = sqrt(a);
            }
            else return NAN;

            st[sp++] = r;
        }
        else if (is_operator(tok)) {
            if (sp < 2) return NAN;
            float b = st[--sp];
            float a = st[--sp];
            float r;

            switch (tok[0]) {
                case '+': r = a + b; break;
                case '-': r = a - b; break;
                case '*': r = a * b; break;
                case '/': if (b == 0) return NAN; r = a / b; break;
                case '%': if (b == 0) return NAN; r = fmod(a, b); break;
                case '^': r = pow(a, b); break;
                default: return NAN;
            }

            st[sp++] = r;
        }
        else return NAN;
    }

    if (sp != 1) return NAN;
    return st[0];
}


//Jacob Bumbuna <developer@devbumbuna.com>
// 2022

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/queue.h>
#include <math.h>
#include "hesabati.h"


#define HESABATI

enum HESABATI_TOKEN {
    TK_NUMBER,
    TK_DOUBLE,
    TK_FLOAT,
    TK_PLUS,
    TK_MINUS,
    TK_DIVIDE,
    TK_TIMES,
    TK_PARENOPEN,
    TK_PARENCLOSE,
    TK_END,
    TK_ERR,
};

#define HESABATI_ERR_OK                                     0
#define HESABATI_PERR_COMPLEX                   1
#define HESABATI_PERR_UNEXPECTED_TOKEN       2
#define HESABATI_PERR_NO_CLOSEPAREN   3
#define HESABATI_LERR                                           -1 

const char *const TK2STR[] = {
    "TK_PLUS",
    "TK_MINUS",
    "TK_DIVIDE",
    "TK_TIMES",
    "TK_NUMBER",
    "TK_DOUBLE",
    "TK_FLOAT",
    "TK_PARENOPEN",
    "TK_PARENCLOSE",
    "TK_END",
};

//tokenizer
const char    *gl_lexeme;
hnum_t           g_num;
hfloat_t          g_float;
hdouble_t     g_double;
const char  *gl_input;
const char  *gl_input_ptr;
int                     g_hesabati_error = HESABATI_ERR_OK;

enum HESABATI_TOKEN
tokenize() {
    enum HESABATI_TOKEN token = TK_END;
    while(1 && token == TK_END) {
        gl_lexeme = gl_input_ptr;
        switch(*gl_input_ptr) {
            case  0:        return TK_END;
            case ' ':
            case '\t':
            case '\n':     gl_input_ptr++; break;
            case '+':       token = TK_PLUS; break;
            case '-':        token = TK_MINUS; break;
            case '*':        token = TK_TIMES; break;
            case '/':         token = TK_DIVIDE; break;
            case '(':         token = TK_PARENOPEN; break;
            case ')':         token = TK_PARENCLOSE; break;
            default: {
                int is_double = 0;
                while(isdigit(*gl_input_ptr)) {
                    gl_input_ptr++;
                }
                if(*gl_input_ptr == '.') {
                    is_double = 1;
                    gl_input_ptr++;
                }
                if(is_double) {
                    while(isdigit(*gl_input_ptr)) {
                        gl_input_ptr++;
                    }
                }
                if(gl_lexeme == gl_input_ptr) {
                  g_hesabati_error = HESABATI_LERR;
                    return TK_ERR;
                } else {
                    char *temp = strndup(gl_lexeme, gl_input_ptr - gl_lexeme);
                    if(is_double) {
                        token = TK_DOUBLE;
                        g_double = strtod(temp, NULL);
                    } else {
                        token = TK_NUMBER;
                        g_num = strtol(temp, NULL, 10);
                    }
                    free(temp);
                    gl_input_ptr--;
                }
            } //end default
        } //end switch
    }//end while
    gl_input_ptr++;
    return token;
}

int
tokenize_test(char *input, enum HESABATI_TOKEN exptected_tokens[]) {
    gl_input = gl_input_ptr = input;
    int i = 0;
    enum HESABATI_TOKEN tk;
    while((tk = tokenize()) != TK_END) {
        assert(tk == exptected_tokens[i++]);
    };
    return 0;
}

//parser
//grammer
/**
 * AMBIGUOUS & LEFT-RECURSIVE  GRAMMAR
 * 1. hesabati  -> e TK_END
 * 2. e -> e TK_PLUS e
 * 3.        |e TK_MINUS e
 * 4.        |e TK_TIMES e
 *  5.       |e TK_DIVIDE e
 *  6.       |TK_PARENOPEN e TK_PARENCLOSE
 *  7.       |TK_NUMBER
 *   8.      |TK_DOUBLE
 * 
 *  UNAMBIGUOUS & RIGHT-RECURSIVE GRAMMAR
 * 0. hesabati -> plus TK_END
 * 1. plus ->sub plus'
 * 2. plus' -> TK_PLUS minus plus'
 * 3.     | EPSILON
 * 4. minus ->  times minus'
 * 5. minus' -> TK_MINUS times minus'
 * 6.       | EPSILON
 * 7. times -> divide times'
 * 8. times' -> TK_TIMES divide times'
 * 9.       | EPSILON
 * 10.divide -> factor divide'
 * 11.divide' -> TK_DIVIDE factor divide'
 * 12.      | EPSILON
 * 13.factor -> TK_PARENOPEN e TK_PARENCLOSE
 * 14.          | TK_NUMBER
 * 15.          | TK_DOUBLE
 **/
enum HESABATI_TOKEN gp_current_token;
#define ADVANCE() (gp_current_token = tokenize())
#define MATCH(x) (gp_current_token == x)

struct HESABATI_PARSENODE {
    enum HESABATI_TOKEN type;
    union {
        hdouble_t dval;
        hnum_t      ival;
        hfloat_t     fval;
    };
    struct HESABATI_PARSENODE *lchild;
    struct HESABATI_PARSENODE *rchild;
    //for use with the STAILQ macros
    STAILQ_ENTRY(HESABATI_PARSENODE) HESABATI_PARSENODE;
};

//if no allocator is present use stdlib for heap allocation
//and deallocation
#define hesabati_malloc(sz) malloc(sz)
#define hesabati_calloc(sz, n) calloc(sz, n)
#define hesabati_free(p) free(p)

typedef struct HESABATI_PARSENODE* htree_ptr_t; 

STAILQ_HEAD(qdel, HESABATI_PARSENODE);
void
hesabati_cuttree(htree_ptr_t tree) {
    if(!tree) return;
    struct qdel qu;
    struct qdel *qu_ptr = &qu;
    STAILQ_INIT(qu_ptr);
    STAILQ_INSERT_TAIL(qu_ptr, tree, HESABATI_PARSENODE);
    while(!STAILQ_EMPTY(qu_ptr)) {
        tree = STAILQ_FIRST(qu_ptr);
        STAILQ_REMOVE_HEAD(qu_ptr, HESABATI_PARSENODE);
        if(tree->lchild) {
            STAILQ_INSERT_TAIL(qu_ptr, tree->lchild, HESABATI_PARSENODE);
        }
        if(tree->rchild) {
            STAILQ_INSERT_TAIL(qu_ptr, tree->rchild, HESABATI_PARSENODE);
        }
        hesabati_free(tree);
    }
}

htree_ptr_t hesabati_plus();
htree_ptr_t hesabati_minus();
htree_ptr_t hesabati_times();
htree_ptr_t hesabati_divide();
htree_ptr_t hesabati_factor();

//production 1
htree_ptr_t
hesabati_parse(const char *expression) {
    gl_input = gl_input_ptr = expression;
    ADVANCE();
    htree_ptr_t root = NULL;
    if(!MATCH(TK_END)) {
        root = hesabati_plus();
        if(!MATCH(TK_END)) {
            g_hesabati_error = HESABATI_PERR_UNEXPECTED_TOKEN;
        }
    } 
    if(g_hesabati_error) {
        hesabati_cuttree(root);
    }
    return root;
}

htree_ptr_t
hesabati_plus() {
    htree_ptr_t p = hesabati_minus();
    while(MATCH(TK_PLUS) && p) {
        ADVANCE();
        htree_ptr_t np = hesabati_malloc(sizeof(*np));
        np->type = TK_PLUS;
        np->lchild = p;
        if(!(np->rchild = hesabati_minus())) {
            hesabati_free(np);
            hesabati_cuttree(p);
            return NULL;
        }
        p = np;
    }
    return p;
}

htree_ptr_t
hesabati_minus() {
    htree_ptr_t p = hesabati_times();
    while(MATCH(TK_MINUS) && p) {
        ADVANCE();
        htree_ptr_t np = hesabati_malloc(sizeof(*np));
        np->type = TK_MINUS;
        np->lchild = p;
        if(!(np->rchild = hesabati_times())) {
            hesabati_free(np);
            hesabati_cuttree(p);
            return NULL;
        };
        p = np;
    }
    return p;
}

htree_ptr_t
hesabati_times() {
    htree_ptr_t p = hesabati_divide();
    while(MATCH(TK_TIMES) && p) {
        ADVANCE();
        htree_ptr_t np = hesabati_malloc(sizeof(*np));
        np->type = TK_TIMES;
        np->lchild = p;
        if(!(np->rchild = hesabati_divide())) {
            hesabati_free(np);
            hesabati_cuttree(p);
            return NULL;
        }
        p = np;
    }
    return p;
}

htree_ptr_t
hesabati_divide() {
    htree_ptr_t p = hesabati_factor();
    while(MATCH(TK_DIVIDE) && p) {
        ADVANCE();
        htree_ptr_t np = hesabati_malloc(sizeof(*np));
        np->type = TK_DIVIDE;
        np->lchild = p;
        if(!(np->rchild = hesabati_factor())) {
            hesabati_free(np);
            hesabati_cuttree(p);
            return NULL;
        }
        p = np;
    }
    return p;
}

htree_ptr_t
hesabati_factor() {
    if(MATCH(TK_PARENOPEN)) {
        ADVANCE();
        htree_ptr_t n = hesabati_plus();
        if(!n) {
            return NULL;
        }
        if(!MATCH(TK_PARENCLOSE)){
            hesabati_free(n);
            g_hesabati_error = HESABATI_PERR_NO_CLOSEPAREN;
            return NULL;
        }
        ADVANCE();
        return n;
    }
    htree_ptr_t n = hesabati_calloc(1,sizeof(*n));
    switch((n->type = gp_current_token)) {
        case TK_NUMBER: n->ival = g_num; break;
        case TK_DOUBLE: n->dval = g_double; break;
        default: {
            hesabati_free(n);
            g_hesabati_error = HESABATI_PERR_UNEXPECTED_TOKEN;
            return NULL;
        }
    }
    ADVANCE();
    return n;
}

#ifdef HESABATI_DEBUG
STAILQ_HEAD(queue_nodes, HESABATI_PARSENODE);
void
hesabati_printtree(htree_ptr_t root) {
    if(!root) return;
    struct queue_nodes q;
    STAILQ_INIT(&q);
    STAILQ_INSERT_TAIL(&q, root, HESABATI_PARSENODE);
    int i = 1;
    while(!STAILQ_EMPTY(&q)) {
        root = STAILQ_FIRST(&q);
        STAILQ_REMOVE_HEAD(&q, HESABATI_PARSENODE);
        if(root->lchild) {
            STAILQ_INSERT_TAIL(&q, root->lchild, HESABATI_PARSENODE);
        }
        if(root->rchild) {
            STAILQ_INSERT_TAIL(&q, root->rchild, HESABATI_PARSENODE);
        }
        for(int k = 0, j = log2(i++); k < j; k++) {
            putchar_unlocked('-');
        }
        printf("%s", TK2STR[root->type]);
        if(root->type == TK_NUMBER) {
            printf(" {val = %lu}", root->ival);
        }
        if(root->type == TK_DOUBLE) {
            printf(" {val =%f", root->dval);
        }
        printf("\n");
    }
}
#endif //HESABATI_DEBUG

// interpreter
#define HESABATI_STACK_SIZE_IN_BYTES 10240 //10KB
typedef unsigned char hbyte;
hbyte gi_stack[HESABATI_STACK_SIZE_IN_BYTES];
hbyte *gi_stack_top = gi_stack; //upward growing, post-incremental push, pre-decremental pop
hbyte gi_stack_types[HESABATI_STACK_SIZE_IN_BYTES/4];
hbyte *gi_stack_types_top = gi_stack_types;
#define GI_STACK_END (&gi_stack[HESABATI_STACK_SIZE_IN_BYTES])

int
hesabati_stack_pushnum(hnum_t n) {
    if(gi_stack_top > (GI_STACK_END-sizeof(hnum_t))) {
        return g_hesabati_error = HESABATI_PERR_COMPLEX;
    }
    *(hnum_t*)gi_stack_top = n;
    *gi_stack_types_top++ = TK_NUMBER;
    gi_stack_top += sizeof(hnum_t);
    return 0;
}

hnum_t
hesabati_stack_popnum() {
    gi_stack_top -= sizeof(hnum_t);
    gi_stack_types_top--;
    return *(hnum_t*)gi_stack_top;
}

int
hesabati_stack_pushd(hdouble_t n) {
    if(gi_stack_top > (GI_STACK_END-sizeof(hdouble_t))) {
        return g_hesabati_error = HESABATI_PERR_COMPLEX;
    }
    *(hdouble_t*)gi_stack_top = n;
    *gi_stack_types_top++ = TK_DOUBLE;
    gi_stack_top += sizeof(hdouble_t);
    return 0;
}

hdouble_t
hesabati_stack_popd() {
    gi_stack_top -= sizeof(hdouble_t);
    gi_stack_types_top--;
    return *(hdouble_t*)gi_stack_top;
}

void
hesabati_double_arith(enum HESABATI_TOKEN t) {
    hdouble_t op1, op2;
    op2 = hesabati_stack_popd();
    op1 = hesabati_stack_popd();
    switch(t) {
        case TK_PLUS: hesabati_stack_pushd(op1+op2); break;
        case TK_MINUS: hesabati_stack_pushd(op1-op2); break;
        case TK_TIMES: hesabati_stack_pushd(op1*op2); break;
        case TK_DIVIDE: hesabati_stack_pushd(op1/op2); break;
        //Not reachable
        default: break;
    }
}

void
hesabati_num_arith(enum HESABATI_TOKEN t) {
    hnum_t op1, op2;
    op2 = hesabati_stack_popnum();
    op1 = hesabati_stack_popnum();
    switch(t) {
        case TK_PLUS: hesabati_stack_pushnum(op1+op2); break;
        case TK_MINUS: hesabati_stack_pushnum(op1-op2); break;
        case TK_TIMES: hesabati_stack_pushnum(op1*op2); break;
        case TK_DIVIDE: hesabati_stack_pushnum(op1/op2); break;
        //Not reachable
        default: break;
    }
}

void 
hesabati_eval(htree_ptr_t t) {
    //abort evaluation if an error is pending
    if(g_hesabati_error || t == NULL) return;
    switch(t->type) {
        case TK_NUMBER: hesabati_stack_pushnum(t->ival); break;
        case TK_DOUBLE: hesabati_stack_pushd(t->dval); break;
        default: {
            hesabati_eval(t->lchild);
            hesabati_eval(t->rchild);
            if(*(gi_stack_types_top-1) != *(gi_stack_types_top-2) ) {
                //type promotion
                if(*(gi_stack_types_top-1) == TK_NUMBER) {
                    hesabati_stack_pushd( (hdouble_t) hesabati_stack_popnum());
                } else {
                    hdouble_t temp = hesabati_stack_popd();
                    hesabati_stack_pushd((hdouble_t) hesabati_stack_popnum());
                    hesabati_stack_pushd(temp);
                }
            }
             if(*(gi_stack_types_top-1) == TK_DOUBLE) {
                    //double aritmetic
                    hesabati_double_arith(t->type);
                } else {
                    hesabati_num_arith(t->type);
                }
        }
    }
}

int
hesabati(const char *expression, int *type, void **store) {
    //reset interpreter
    gi_stack_top = gi_stack;
    gi_stack_types_top = gi_stack_types;
    g_hesabati_error = 0;
    htree_ptr_t ast;
    ast = hesabati_parse(expression);
    hesabati_eval(ast);
    *type = *gi_stack_types;
    *store = gi_stack;
    hesabati_cuttree(ast);
    return g_hesabati_error;
}

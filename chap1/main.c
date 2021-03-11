#include <stdio.h>
#include "util.h"
#include "slp.h"
#include "prog1.h"
int expListSolver(A_expList list){
    if (list->kind == A_lastExpList){
        return 1;
    } else{
        return 1 + expListSolver(list->u.pair.tail);
    }
}
int maxargs(A_stm stm){
    switch (stm->kind){
        case A_compoundStm:{
            int left = maxargs(stm->u.compound.stm1);
            int right = maxargs(stm->u.compound.stm2);
            return left > right ? left : right;
        } case A_assignStm:{
            A_exp exp = stm->u.assign.exp;
            if (exp->kind == A_eseqExp){
                return maxargs(exp->u.eseq.stm);
            } else{
                return 0;
            }
        } case A_printStm:{
            return expListSolver(stm->u.print.exps);
        } default:{
            return 0;
        }
    }
}

typedef struct table *Table_;
struct table{
    string id;
    int value;
    Table_ tail;
};
Table_ Table(string id, int value, Table_ tail){
    Table_ t = checked_malloc(sizeof(*t));
    t->id = id;
    t->value = value;
    t->tail = tail;
    return t;
}
typedef struct IntAndTable *iTable_;
struct IntAndTable {
    int value;
    Table_ t;
};
iTable_ iTable(int i, Table_ t){
    iTable_ it = checked_malloc(sizeof(*it));
    it->value = i;
    it->t = t;
}
iTable_ interpExp(A_exp e, Table_ t);
Table_ interpExpList(A_expList list, Table_ t);

Table_ interpStm(A_stm s, Table_ t){
    switch(s->kind){
        case A_compoundStm:{
            t = interpStm(s->u.compound.stm1, t);
            t = interpStm(s->u.compound.stm2, t);
            return t;
        } case A_assignStm:{
            iTable_ it = interpExp(s->u.assign.exp, t);
            t = Table(s->u.assign.id, it->value, it->t);
            return t;
        } case A_printStm:{
            t = interpExpList(s->u.print.exps, t);
            printf("\n");
            return t;
        } default:{
            return t;
        }
    }
}

int ID_INVALID = 0;

int lookup(Table_ t, string key){
    if(t == NULL){
        ID_INVALID = 1;
        return -1;
    }
    if(t->id == key){
        return t->value;
    } else{
        return lookup(t->tail, key);
    }
}

iTable_ interpExp(A_exp e, Table_ t){
    switch(e->kind){
        case A_idExp:{
            ID_INVALID = 0;
            int value = lookup(t, e->u.id);
            if(ID_INVALID){
                printf("Identifier %s doesn't exist!\n", e->u.id);
            }
            return iTable(value, t);
        } case A_numExp:{
            return iTable(e->u.num, t);
        } case A_opExp:{
            A_binop op = e->u.op.oper;
            iTable_ left = interpExp(e->u.op.left, t);
            iTable_ right = interpExp(e->u.op.right, t);
            switch(op){
                case A_plus:{
                    return iTable(left->value + right->value, t);
                }
                case A_minus:{
                    return iTable(left->value - right->value, t);
                }
                case A_times:{
                    return iTable(left->value * right->value, t);
                }
                case A_div:{
                    return iTable(left->value / right->value, t);
                } default:{
                    return NULL;
                }
            }
        } case A_eseqExp:{
            t = interpStm(e->u.eseq.stm, t);
            return interpExp(e->u.eseq.exp, t);
        } default:{
            return NULL;
        }
    }
}

Table_ interpExpList(A_expList list, Table_ t){
    switch(list->kind){
        case A_lastExpList:{
            iTable_ it = interpExp(list->u.last, t);
            printf("%d ", it->value);
            return it->t;
        } case A_pairExpList:{
            iTable_ it = interpExp(list->u.pair.head, t);
            printf("%d ", it->value);
            t = interpExpList(list->u.pair.tail, it->t);
            return t;
        } default:{
            return t;
        }
    }
}

void interp(A_stm stm){
    interpStm(stm, NULL);
}

int main(){
    A_stm p = prog();
    printf("%d\n", maxargs(p));
    interp(p);
    return 0;
}
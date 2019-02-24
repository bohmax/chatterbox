#include <Hashtable.h>

int eliminazione(hash *table,char*nome){
    size_t i=get_hash(nome);
    int res=0;
    table[i].root=eliminaalb(table[i].root, nome,&res);
    if(!res){
        return OP_NICK_UNKNOWN;
    }
    return OP_OK;
}

int inserisciconindice(hash *table,char*name,int i, int dec){
    int res=0;
    table[i].root=inseriscialb(table[i].root, name,&res, dec);
    if(res){
        return OP_NICK_ALREADY;
    }
    return OP_OK;

}

int inserisci(hash *table,char nome[]){
    size_t i=get_hash(nome);
    int res=0;
    table[i].root=inseriscialb(table[i].root, nome,&res,0);
    if(res){
        return OP_NICK_ALREADY;
    }
    return OP_OK;
}

struct node *cerca(hash *table,char*nome){
    size_t i=get_hash(nome);
    return cercalb(table[i].root, nome);
}

hash *create(){
    hash* vet=NULL;
    ERRORSYSHEANDLER(vet, malloc(NUMMAX*sizeof(hash)), NULL, "malloc struttura dati\n")
    for(int i=0;i<NUMMAX;i++)
        vet[i].root=NULL;
    return vet;
}



hash *freeh(hash* table, int nmax){
    for(int i=0;i<NUMMAX;i++){
        if(table[i].root)
            freeb(table[i].root,nmax);
    }
    free(table);
    return NULL;
}

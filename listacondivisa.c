#include <listacondivisa.h>

lista* inseriscil(lista* root,int connfd,message_t* mem,decision decidi){
    lista* new=NULL;
    ERRORSYSHEANDLER(new, malloc(sizeof(lista)), NULL, "malloc list");
    new->fd=connfd;
    if(mem)
        new->messaggio=mem;
    else new->messaggio=NULL;
    new->decisione=decidi;
    new->prec=NULL;
    if(root==NULL){ //non ci sono elementi
        new->next=NULL;
        new->last=new;
    }
    else{
        new->next=root;
        root->prec=new;
        new->last=root->last;
        root->last=NULL;
    }
    return new;
}

gestisce estrai(lista *root){
    gestisce send;
    if(root->last==root){
        send.fd=root->fd;
        if(root->messaggio) send.gestisci=root->messaggio;
        else send.gestisci=NULL;
        send.decisione=root->decisione;
        free(root);
        root=NULL;
        return send;
    }
    else{
        send.fd=root->last->fd;
        if(root->last->messaggio) send.gestisci=root->last->messaggio;
        else send.gestisci=NULL;
        send.decisione=root->last->decisione;
    }
    lista* temp=root->last;
    temp->prec->next=NULL;
    root->last=temp->prec;
    free(temp);
    temp=NULL;
    return send;
}

opback estraib(lista *root){
    opback send;
    if(root->last==root){
        send.fd=root->fd;
        send.operazione=root->decisione;
        free(root);
        root=NULL;
        return send;
    }
    send.fd=root->last->fd;
    send.operazione=root->last->decisione;
    lista* temp=root->last;
    temp->prec->next=NULL;
    root->last=temp->prec;
    free(temp);
    temp=NULL;
    return send;
}

void freecondl(lista* lib){
    if(lib->next==NULL) free(lib);
    else{
        freecondl(lib->next);
        free(lib);
    }
}

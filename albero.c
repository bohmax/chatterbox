#include <albero.h>

avl *nuovo(char* name,int dec){
    avl *nodo=NULL;
    ERRORSYSHEANDLER(nodo, (calloc(1,sizeof(avl))), NULL, "Impossibile allocare albero");
    memcpy(nodo->nickname,name,strlen(name)+1);
    if(dec)
        nodo->connesso=ITSGROUP;
    else
        nodo->connesso=ITSUSER;
    nodo->group=NULL;
    nodo->nmessaggi_giacenti=-1;
    nodo->history=NULL;
    nodo->left=NULL;
    nodo->right=NULL;
    nodo->altezza=1;
    return nodo;
}

int newaltezza(int a,int b){
    if(a>b)
        return ++a;
    return ++b;
}

avl *rotazioneadestra(avl *padre){ //ritorna il nuovo padre destro
    avl *sinistra=padre->left;
    avl *destra1=sinistra->right;
    sinistra->right=padre; //gestisco la rotazione
    padre->left=destra1;
    //aggiornamento altezza
    padre->altezza=newaltezza(HEIGHT(padre->left),HEIGHT(padre->right));
    sinistra->altezza=newaltezza(HEIGHT(sinistra->left),HEIGHT(sinistra->right));
    return sinistra;
}

avl *rotazioneasinistra(avl *padre){ //ritorna il nuovo padre sinistro
    avl *destra=padre->right;
    avl *sinistra1=destra->left;
    destra->left=padre; //gestisco la rotazione
    padre->right=sinistra1;
    //aggiornamento altezza
    padre->altezza=newaltezza(HEIGHT(padre->left),HEIGHT(padre->right));
    destra->altezza=newaltezza(HEIGHT(destra->left),HEIGHT(destra->right));
    return destra;
}

int isBalance(avl *x){
    if(x==NULL)
        return 0;
    return (HEIGHT(x->left)) - (HEIGHT(x->right));
}

avl *inseriscialb(avl *nodo, char name[],int* riuscita, int dec){
    if(nodo==NULL)
        return nuovo(name, dec);
    int i=strcmp(name,nodo->nickname);
    if(i<0)
        nodo->left=inseriscialb(nodo->left, name,riuscita,dec);
    else if(i>0)
        nodo->right=inseriscialb(nodo->right, name,riuscita,dec);
    else{ //nomi uguali o non sono presenti
        *riuscita=1;
        return nodo;
    }
    nodo->altezza=newaltezza(HEIGHT(nodo->left),(HEIGHT(nodo->right)));
    
    int bilanciato=isBalance(nodo);
    if(bilanciato>1){
        int y=strcmp(name,nodo->left->nickname);
        if(y<0)
            return rotazioneadestra(nodo);
        if(y>0){
            nodo->left=rotazioneasinistra(nodo->left);
            return rotazioneadestra(nodo);
        }
    }
    if(bilanciato<-1){
        int y=strcmp(name,nodo->right->nickname);
        if(y>0)
            return rotazioneasinistra(nodo);
        if(y<0){
            nodo->right=rotazioneadestra(nodo->right);
            return rotazioneasinistra(nodo);
        }
    }
    return nodo;
}

avl *minimoval(avl* nodo){
    avl* corrente = nodo;
    
    while (corrente->left != NULL)
        corrente = corrente->left;
    
    return corrente;
}

avl* eliminaalb(avl* root, char *name,int* riuscita){
    if (root == NULL){
        return root;
    }
    int i=strcmp(name,root->nickname);
    if (i<0)
        root->left = eliminaalb(root->left, name,riuscita);
    else if(i>0)
        root->right = eliminaalb(root->right, name,riuscita);
    else
    {
        if( (root->left == NULL) || (root->right == NULL) )
        {
            avl*temp = (root->left)? root->left :root->right;
            
            if (temp == NULL){
                temp = root;
                root = NULL;
            }
            else // un figlio
                *root = *temp;
            *riuscita=1;
            if(temp->history){
                int nmax=configurazioni->dim[MaxHistMsgs]-1;
                while(nmax>=0){
                    if(temp->history[nmax].data.buf)
                        free(temp->history[nmax].data.buf);
                    nmax--;
                }
                free(temp->history);
            }
            freeb(temp->group,configurazioni->dim[MaxHistMsgs]-1);
            free(temp);
        }
        else
        {
            avl *temp = minimoval(root->right);
            strncpy(root->nickname,temp->nickname,strlen(temp->nickname)+1);
            root->connesso=temp->connesso;
            root->history=temp->history;
            root->nmessaggi_giacenti=temp->nmessaggi_giacenti;
            root->group=temp->group;
            root->right = eliminaalb(root->right, temp->nickname,riuscita);
        }
    }
    if (root == NULL)
        return root;
    
    root->altezza = newaltezza(HEIGHT(root->left),HEIGHT(root->right));
    
    //se sono sbilanciato ribilancio
    int balance = isBalance(root);
    
    // sinistra sinistra
    if (balance > 1 && isBalance(root->left) >= 0)
        return rotazioneadestra(root);
    
    // sinistra destra
    if (balance > 1 && isBalance(root->left) < 0)
    {
        root->left =  rotazioneasinistra(root->left);
        return rotazioneadestra(root);
    }
    
    // Destra destra
    if (balance < -1 && isBalance(root->right) <= 0)
        return rotazioneasinistra(root);
    
    // destra sinistra
    if (balance < -1 && isBalance(root->right) > 0)
    {
        root->right = rotazioneadestra(root->right);
        return rotazioneasinistra(root);
    }
    
    return root;
}

void ordina(avl* root){
    if(root!=NULL){
        ordina(root->left);
        printf("%s ",root->nickname);
        ordina(root->right);
    }
}

void freeb(avl *root, int nmax){
    if(root){
        freeb(root->right,nmax);
        freeb(root->left,nmax);
        if(root->history){
            while(nmax>=0){
                if(root->history[nmax].data.buf){
                    free(root->history[nmax].data.buf);
                    root->history[nmax].data.buf=NULL;
                }
                nmax--;
            }
            free(root->history);
        }
        if(root->group)
            freeb(root->group,nmax);
        free(root);
    }
}

avl* cercalb(avl* nodo,char* name){
    if(nodo==NULL) return NULL;
    int i=strcmp(name, nodo->nickname);
    if(i==0)
        return nodo;
    else if(i>0)
        return cercalb(nodo->right,name);
    else
        return cercalb(nodo->left,name);
}

avl* cercaoff(avl* nodo,char* name){
    if(nodo==NULL) return NULL;
    int i=strcmp(name, nodo->nickname);
    if(i==0){
        nodo->connesso=-1;
        return nodo;
    }
    else if(i>0)
        return cercaoff(nodo->right,name);
    else
        return cercaoff(nodo->left,name);
}

//online
online *nuovoo(int val,char name[33]){
    online *nodo=NULL;
    ERRORSYSHEANDLER(nodo, (malloc(sizeof(online))), NULL, "Impossibile allocare albero");
    nodo->fd=val;
    memcpy(nodo->nickname,name,MAX_NAME_LENGTH+1);
    nodo->left=NULL;
    nodo->right=NULL;
    nodo->altezza=1;
    return nodo;
}

online *rotazioneadestrao(online *padre){ //ritorna il nuovo padre destro
    online *sinistra=padre->left;
    online *destra1=sinistra->right;
    sinistra->right=padre; //gestisco la rotazione
    padre->left=destra1;
    //aggiornamento altezza
    padre->altezza=newaltezza(HEIGHT(padre->left),HEIGHT(padre->right));
    sinistra->altezza=newaltezza(HEIGHT(sinistra->left),HEIGHT(sinistra->right));
    return sinistra;
}

online *rotazioneasinistrao(online *padre){ //ritorna il nuovo padre sinistro
    online *destra=padre->right;
    online *sinistra1=destra->left;
    destra->left=padre; //gestisco la rotazione
    padre->right=sinistra1;
    //aggiornamento altezza
    padre->altezza=newaltezza(HEIGHT(padre->left),HEIGHT(padre->right));
    destra->altezza=newaltezza(HEIGHT(destra->left),HEIGHT(destra->right));
    return destra;
}

int isBalanceo(online *x){
    if(x==NULL)
        return 0;
    return (HEIGHT(x->left)) - (HEIGHT(x->right));
}

online *minimovalo(online* nodo){
    online* corrente = nodo;
    
    while (corrente->left != NULL)
        corrente = corrente->left;
    
    return corrente;
}

online *inseriscialbo(online *nodo,int val,char name[]){
    if(nodo==NULL)
        return nuovoo(val,name);
    int i=val-nodo->fd;
    if(i<0)
        nodo->left=inseriscialbo(nodo->left, val,name);
    else if(i>0)
        nodo->right=inseriscialbo(nodo->right, val,name);
    else{ //nomi uguali o non sono presenti
        return nodo;
    }
    nodo->altezza=newaltezza(HEIGHT(nodo->left),(HEIGHT(nodo->right)));
    
    int bilanciato=isBalanceo(nodo);
    if(bilanciato>1){
        int y=val-nodo->fd;
        if(y<0)
            return rotazioneadestrao(nodo);
        if(y>0){
            nodo->left=rotazioneasinistrao(nodo->left);
            return rotazioneadestrao(nodo);
        }
    }
    if(bilanciato<-1){
        int y=val-nodo->fd;
        if(y>0)
            return rotazioneasinistrao(nodo);
        if(y<0){
            nodo->right=rotazioneadestrao(nodo->right);
            return rotazioneasinistrao(nodo);
        }
    }
    return nodo;
}

int nameo(online* t,char *tmp,int sum){ //nomi utenti online
    if(t==NULL) return 0;
    if(t->right==NULL && t->left==NULL){
        memcpy(tmp+sum,t->nickname,MAX_NAME_LENGTH+1);
        return sum+33;
    }
    if(t->left!=NULL)sum=nameo(t->left,tmp,sum);
    if(t->right!=NULL)sum=nameo(t->right,tmp,sum);
    memcpy(tmp+sum,t->nickname,MAX_NAME_LENGTH+1);
    return sum+33;
}

online* eliminaalbo(online* root,int val,int* err){
    if (root == NULL){
        return root;
    }
    int i=val-root->fd;
    if (i<0)
        root->left = eliminaalbo(root->left, val,err);
    else if(i>0)
        root->right = eliminaalbo(root->right, val,err);
    else
    {
        if( (root->left == NULL) || (root->right == NULL) )
        {
            online*temp = (root->left)? root->left :root->right;
            if(!nametodel){
                ERRORSYSHEANDLER(nametodel,malloc(sizeof(char)*(MAX_NAME_LENGTH+1)),NULL,"no asse offline")
                memcpy(nametodel,root->nickname,MAX_NAME_LENGTH+1);
            }
            if (temp == NULL){
                temp = root;
                root = NULL;
            }
            else // un figlio
                *root = *temp;
            *err=1;
            free(temp);
            temp=NULL;
        }
        else
        {
            online *temp = NULL;
            temp=minimovalo(root->right);
            ERRORSYSHEANDLER(nametodel,malloc(sizeof(char)*(MAX_NAME_LENGTH+1)),NULL,"no asse offline 1")
            memcpy(nametodel,root->nickname,MAX_NAME_LENGTH+1);
            
            //copia i dati qui
            root->fd=temp->fd;
            memcpy(root->nickname,temp->nickname,MAX_NAME_LENGTH+1);
            
            root->right = eliminaalbo(root->right, temp->fd,err);
        }

    }
    if (root == NULL){
        return root;
    }
    root->altezza = newaltezza(HEIGHT(root->left),HEIGHT(root->right));
    
    //se sono sbilanciato ribilancio
    int balance = isBalanceo(root);
    
    // sinistra sinistra
    if (balance > 1 && isBalanceo(root->left) >= 0)
        return rotazioneadestrao(root);
    
    // sinistra destra
    if (balance > 1 && isBalanceo(root->left) < 0)
    {
        root->left =  rotazioneasinistrao(root->left);
        return rotazioneadestrao(root);
    }
    
    // Destra destra
    if (balance < -1 && isBalanceo(root->right) <= 0)
        return rotazioneasinistrao(root);
    
    // destra sinistra
    if (balance < -1 && isBalanceo(root->right) > 0)
    {
        root->right = rotazioneadestrao(root->right);
        return rotazioneasinistrao(root);
    }
    
    return root;
}

void freeo(online *root){
    if(root!=NULL){
        freeo(root->right);
        freeo(root->left);
        free(root);
    }
}

online* cercalbo(online* nodo,int val){
    if(nodo==NULL) return NULL;
    int i=val-nodo->fd;
    if(i==0)
        return nodo;
    else if(i>0)
        return cercalbo(nodo->right,val);
    else
        return cercalbo(nodo->left,val);
}

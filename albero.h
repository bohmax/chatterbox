//
//  albero.h
//  Server
//
//  Created by Massimo Puddu on 13/06/17.
//  Copyright © 2017 Massimo Puddu 531379. All rights reserved.
//

#ifndef ALBERO
#define ALBERO
#ifndef HEADER_H
#include <Header.h>
#endif
#define HEIGHT(esiste) (esiste==NULL)?0:esiste->altezza
/**
 * @file  albero.h
 * @brief Contiene l'albero utenti online e gli alberi che fanno da adiacenza alla tabella hash e le loro relative funzioni
 *
 */

/**
 *  @struct node
 *  @brief nodo che contiene tutti i dati possibili di un utente
 *
 *  @var nickname del ricevente
 *  @var connesso indica il fd se connesso. Ha valore -1 se non connesso, mentre ha valore -2 se indica un gruppo. Vale -3 se si tratta del capogruppo
 *  @var nmessaggi_giacenti indica in modulo in numero di messaggi giacenti
 *  @var history contiene i messaggi giacenti
 *  @var group utilizzato per memorizzare i nome degli utenti nel caso in cui il nodo sia un gruppo, se il nodo è un utente invece memorizzo i gruppi di cui fa parte
 *  @var altezza la sua altezza all'interno dell'albero
 */
typedef struct node{
    char nickname[MAX_NAME_LENGTH+1];
    int connesso;
    int nmessaggi_giacenti;
    message_t* history;
    struct node *left;
    struct node *right;
    struct node* group;
    int altezza;
}avl;

/**
 *  @struct on
 *  @brief struttura che gestisce gli utenti connessi
 *
 *  @var fd contiene il file descriptor del nickname
 *  @var nickname del ricevente
 *  @var altezza la sua altezza all'interno dell'albero
 */
typedef struct on{
    int fd;
    char nickname[MAX_NAME_LENGTH+1];//nome associato al file descriptor
    struct on* right;
    struct on* left;
    int altezza;
}online;

extern online* connections;///variabile globale contenente la testa della coda
extern char* nametodel;///variabile globale che uso per memorizzarmi il nome di un utente da cancellare

/**
 inserisce elementi nell'avl

 @param nodo     nodo dell'albero
 @param name     nome da inserire nell'albero
 @param riuscita intero che indica se l'operazione è andata a buon fine
 @param dec      utilizzato per capire se devo inserire un gruppo

 @return il padre dell'albero
 */
avl *inseriscialb(avl *nodo, char *name,int* riuscita, int dec);

/**
 funzione ausiliaria utilizzata da inseriscialb per creae una foglia

 @param name nome della parsona da inserire nella foglia
 @param dec  per capire se bisogna crearlo come persona o gruppo

 @return la nuova foglia
 */
avl *nuovo(char *name, int dec);

/**
 permette di dire se l'albero è bilanciato

 @param x albero da analizzare

 @return il fattore di bilanciamento
 */
int isBalance(avl *x);

/**
 permette di trovare il valore min in modo da poter girare l'albero in caso di eliminazione

 @param nodo da esaminare

 @return il nodo col minimo valore
 */
avl *minimoval(avl* nodo);

/**
 elimina la foglia contenente name

 @param root     padre da cui inizia la ricerca
 @param name     nome da eliminare dall'albero
 @param riuscita indica se l'operazione è andata a buon fine

 @return il nuovo root dopo l'eliminazione
 */
avl* eliminaalb(avl* root, char *name,int* riuscita);//l'ultimo parametro è per sapere la riuscita dell'operazione

/**
 fa girare a destra l'albero

 @param padre foglia da cui partire

 @return ritorna il sottoalbero girato a destra
 */
avl *rotazioneadestra(avl *padre);

/**
 fa girare a sinistra l'albero
 
 @param padre foglia da cui partire
 
 @return ritorna il sottoalbero girato a sinistra
 */
avl *rotazioneasinistra(avl *padre);

/**
 libera tutto l'albero
 
 @param root la foglia da cui parte l'eliminazione dell'albero utenti
 */
void freeb(avl *root, int nmax);

/**
 funzione che prende la massima altezza tra due foglie

 @param a altezza della prima foglia
 @param b altezza della seconda foglia

 @return la nuova altezza e ci somma 1.
 */
int newaltezza(int a,int b);

/**
 trova un elemento a partire dal nodo

 @param nodo nodo da cui iniziare la ricerca
 @param name nome da cercare

 @return il nodo interessato, ritorna NULL se nome non era presente nell'albero
 */
avl* cercalb(avl* nodo,char* name);

/**
 trova e setta offline un utente

 @param nodo nodo da cui perte la ricerca
 @param name nome dell'utente da mettere offline

 @return il nodo padre, se non lo trova ritorna NULL
 */
avl* cercaoff(avl* nodo,char* name);

/**
 cerca gli utenti nell'albero online

 @param nodo nodo da cui inizia la ricerca
 @param val  file descriptor da cercare

 @return NULL se la ricerca fallisce, altrimenti ritorna il nodo cercato
 */
online* cercalbo(online* nodo,int val);

/**
 inserisci un file descriptor nell'albero online

 @param nodo nodo da cui parte l'inserimento
 @param val  file descriptor da inserire
 @param name nome abbinato al file descriptor

 @return ritorna il padre dell'albero
 */
online *inseriscialbo(online *nodo, int val,char name[]);

/**
 fa girare a destra l'albero utenti online
 
 @param padre foglia da cui partire
 
 @return ritorna il sottoalbero girato a destra
 */
online *rotazioneadestrao(online *padre);

/**
 fa girare a sinistra l'albero utenti online
 
 @param padre foglia da cui partire
 
 @return ritorna il sottoalbero girato a sinistra
 */
online *rotazioneasinistrao(online *padre);

/**
 memorizza su tmp gli utenti online

 @param t   padre da cui iniziare a prendere il nome degli utenti online da scrivere su tmp
 @param tmp char* in cui si va a memorizzare il nome degli utenti online
 @param sum indica la posizione da cui iniziare a scrivere in tmp

 @return restituisce la posizione dell'ultimo utente scritto in tmp
 */
int nameo(online* t,char *tmp,int sum);

/**
 elimina la foglia contenente val
 
 @param root padre da cui inizia la ricerca
 @param val file descriptor da eliminare dall'albero
 @param err indica se l'operazione è andata a buon fine
 
 @return il nuovo root dopo l'eliminazione
 */
online* eliminaalbo(online* root,int val,int* err);

/**
 libera tutto l'albero

 @param root la foglia da cui parte l'eliminazione dell'albero utenti online
 */
void freeo(online *root);

#endif //ALBERO

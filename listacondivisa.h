//
//  liste.h
//  Server
//
//  Created by Massimo Puddu on 16/06/17.
//  Copyright © 2017 Massimo Puddu 531379. All rights reserved.
//

#ifndef LISTA
#define LISTA
#ifndef HEADER_H
#include <Header.h>
#endif

/**
 * @file  listacondivisa.h
 * @brief Contiene le liste e le strutture dati per la comunicazione tra listener e worker
 *
 */

/**
 *  @struct messaggio
 *  @brief struttura utilizzata soddisfare la richiesta di un client
 *
 *  @var gescisci contiene il messaggio da gestire
 *  @var fd file descriptor di chi manda una richiesta
 *  @var decisione mi aiuta a gestire e capire come comportarmi con le richieste
 */
typedef struct messaggio{
    message_t* gestisci;
    int fd;//file descriptor
    decision decisione;
}gestisce;

/**
 *  @struct opback
 *  @brief struttura per far comuncare il worker con il listerner
 *
 *  @var fd file descriptor interessato
 *  @var operazione può assumere due valore, con 0 il listener deve chiudere il file descriptor, con 1 lo aggiunge al master_set
 */
typedef struct{
    int fd;
    int operazione;
}opback;


/**
 *  @struct nodi
 *  @brief struttura utilizzata per far comunicare il lister col worker o far comunicare diversi worker tra loro
 *
 *  @var fd contiene il messaggio da gestire
 *  @var gescisci contiene il messaggio da gestire
 *  @var decisione mi aiuta a gestire e capire come comportarmi con le richieste
 *  @var next punta all'elemento successivo
 *  @var prec punta all'elemento precedente della lista
 *  @var last utilizzato solo nel root per trovare l'ultimo elemento
 */
typedef struct nodi{
    int fd;
    message_t* messaggio;
    int decisione;
    struct nodi* next;
    struct nodi* prec;
    struct nodi* last;
}lista;

//inserisce nella lista
/**
 inserisce un elemento in lista

 @param root   testa della lista
 @param connfd file descriptor
 @param mem    contiene il messaggio da gestire
 @param decidi mi aiuta a gestire e capire come comportarmi con le richieste

 @return ritorna la testa della lista
 */
lista* inseriscil(lista* root,int connfd,message_t* mem,int decidi);

/**
 astrae un elemento dalla lista

 @param root root della lista

 @return la struttura per soddisfare la richiesta del client
 */
gestisce estrai(lista *root);

/**
 astrae un elemento dalla lista per il listener
 
 @param root root di opback
 
 @return un opback
 */
opback estraib(lista *root);

/**
 libera tutta la lista

 @param lib testa da cui iniziare a eliminare
 */
void freecondl(lista* lib);

#endif //LISTA

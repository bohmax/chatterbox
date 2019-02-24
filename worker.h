//
//  worker.h
//  Server
//
//  Created by Massimo Puddu on 02/08/17.
//  Copyright © 2017 Massimo Puddu 531379. All rights reserved.
//

#ifndef WORKER
#define WORKER
#ifndef HEADER_H
#include <Header.h>
#endif

#define HELP (imtx2)%configurazioni->dim[MaxHistMsgs]
#define MODULARE if(isonline->history[HELP].data.buf) fd=HELP; \
else fd=0; //serve per sapere la posizione in modulo dell'ultimo messaggio

/**
 * @file  worker.h
 * @brief Contiene il thread worker
 *
 */

/**
 thread worker

 @param arg non contiene alcun valore

 @return (void*) 0
 */
void* pool(void *arg);

/**
 aggiona la statistica richiesta

 @param stat statistica da aggiornare
 @param op   a seconda del valore decido se sotrarre o sommare
 */
void updatestats(unsigned long* stat, int op);

/**
 setta offline un utente

 @param conn file descriptor richiesto dall'utente
 */
void setoffline(int conn);

/**
 rimanda a un'altro thread una richiesta, utilizzato per suddividere operazioni pesanti

 @param nickname   da inserire in gestisci
 @param spediscimi messaggio da copiare per un altro thread
 */
void directsend(char* nickname, gestisce spediscimi);

/**
 per spedire un a richiesta a tutto il sottoalbero

 @param padre      padre da cui partire a spedire le richieste
 @param spediscimi operazione da far rimbalzare
 @param i          mi dice se devo star attento a non far rispedire la richiesta allo stesso utente
 */
void sendtolist(avl* padre, gestisce spediscimi, int i);

/**
 crea e spedisce la lista utenti online

 @param txt  serve per sapere a chi scrivere e cosa scrivere
 @param imtx indice per la mutex
 */
void usrlist(gestisce txt,int imtx);

/**
 inserisce una persona in un gruppo, restituisce l'esito dell'operazione

 @param group  nome del gruppo
 @param sender chi iscrivere

 @return notifica se l'operazione è andata a buon fine
 */
int insertusergroup(char* group, char* sender);

#endif /* worker_h */

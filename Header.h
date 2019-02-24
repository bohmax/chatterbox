//
//  Header.h
//  9
//
//  Created by Massimo Puddu on 06/05/17.
//  Copyright © 2017 Massimo Puddu 531379. All rights reserved.
//

#ifndef HEADER_H
#define HEADER_H
#define ERRORSYSHEANDLER(r,c,d,e) if((r=c)==d) { perror(e);exit(errno); }
#define ERRORSYSWRITE(r) if(r<=0 || errno==EPIPE)  {printf(("%d\n"),errno); }
#define SYSHEANDLER(r,d,e) if((r==d) { perror(e);exit(errno); }
#define SYSFREE(r,c,d,e) if((r=c)!=d) { perror(e);exit(errno); }
#define ERRORHEANDLER(r,c,d,e) if((r=c)==d) { fprintf(stderr, e);exit(1); }
#define MEMERROR(r,c,e) if(r==c) { fprintf(stderr, e);exit(1); }
#define SETERROR(r,c,e) if(r!=c) { fprintf(stderr, e);exit(1); }
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <ftw.h> //per cancellare le caerelle
#include <libgen.h> //serve per basename in modo da avere il file dal path

/**
 * @file  Header.h
 * @brief Contiene i principali include utilizzato in tutto il programma, contiene anche la dichiarazione delle variabili globali e diversi tipi enumeratori
 */

typedef enum dim{
    
    /* -------------------------------------------------------------------------- */
    /*                Per la gestione del file di configurazione                  */
    /* -------------------------------------------------------------------------- */
    UnixPath = 0,       ///indica la posizione di unixpath in cofig->path
    DirName  = 1,       ///indica la posizione di dirname in cofig->path
    StatFileName = 2,   ///indica la posizione di statfilename in cofig->path
    ThreadsInPool = 0,  ///indica la posizione di threadinpool in cofig->dim
    MaxConnections = 1, ///indica la posizione di maxconnection in cofig->dim
    MaxFileSize = 2,    ///indica la posizione di Maxfilesize in cofig->dim
    MaxHistMsgs = 3,    ///indica la posizione di MaxHistmsgs in cofig->dim
    MaxMsgSize = 4      ///indica la posizione di MaxMsgSize in cofig->dim
}conf;

typedef enum {
    
    /* -------------------------------------------------------------------------- */
    /*      Per capire se l'utente è un gruppo oppure no                          */
    /* -------------------------------------------------------------------------- */
    ITSGROUPOWNER =-3, ///si tratta del creatore del gruppo
    ITSGROUP      =-2, ///l'utente è un gruppo
    ITSUSER       =-1  ///l'utente è una persone
} users;

typedef enum {
    
    /* -------------------------------------------------------------------------- */
    /*      Decisioni che il server deve compiere per capire cosa fare            */
    /* -------------------------------------------------------------------------- */
    Connettilo      =-2, ///serve per capire se può un client connettersi al server oppure no ed eseguire op
    Exec_op         =-1, ///per far eseguire un op da un client già connesso
    SendMessThreas  =2,  ///suddivide una richiesta di invio messaggio a un'altro thread
    SendFileThread  =3,  ///suddivide op di invio testo
    GetPrevInThread =4,  ///suddivide l'invio di messaggi di getprevmsg
    GroupThread     =5,  ///si usa per suddividire operazioni dirette dei gruppi
    DelUserGroup    =6   ///per cancellare l'utente deregistrato da tutti i suoi gruppi
} decision;

#ifndef MESSAGE_H_
#include <message.h>
#endif
#ifndef CONNECTIONS_H_
#include <connections.h>
#endif
#ifndef ALBERO
#include <albero.h>
#endif
#ifndef LISTA
#include <listacondivisa.h>
#endif
#ifndef HASH
#include <Hashtable.h>
#endif
#ifndef CONFIG
#include <Configurazione.h>
#endif
#ifndef MEMBOX_STATS_
#include <stats.h>
#endif

#define NUMMAX 2000 //numero scelto perchè nel pdf c'era scritto di gestire un centinaio di utenti registrati
#define DIV 50 //numero in cui divido le lock della tabella

//dichiarazioni variabili globali
extern struct configuration* configurazioni; ///variabile globale che contiene le informazioni del file configurazioni
extern char* nametodel;///per cancellare utente online
extern struct nodi* root;///lista da select a worker
extern struct nodi* back;///lista da worker a set
extern int nutentionline;///numero di utenti online
extern int nconnessioni;///numero di connessioni
extern struct table* userstable; ///tabella hash che memorizza gli utenti registrati e linka a tutte le rispettive strutture utente
extern struct on* connections;///lista che controlla gli utenti online
extern pthread_mutex_t mtxlist;///mutex lista per la comunicazione dei thread
extern pthread_mutex_t mtxtoselect;///mutex per la select
extern pthread_mutex_t mtxhead[NUMMAX/DIV];///mutex per la tabella hash nelle operazioni di registrazione e connessione partizionata facendo 2000/DIV
extern pthread_mutex_t mtxwrite[NUMMAX/DIV];///mutex parallelo a mutexhead per la scrittura nei socket
extern pthread_mutex_t mtxconn;///mutex utenti connessi
extern pthread_mutex_t mtxstat;///mutex per le statistiche
extern pthread_mutex_t mtxnconnessioni;///mutex per il numero di connessioni
extern pthread_cond_t cond;///condizione per produttore consumatore
extern volatile sig_atomic_t flag;///per chiudere tutti i thread


/**
 funzione hash
 
 @param cp nome da utilizzare per la tabella hash
 
 @return la posizione in cui memorizzare un utente nella tabella hash
 */
static inline size_t get_hash(const char* cp){
    size_t hash = 0x811c9dc5;
    while (*cp) {
        hash ^= (unsigned char) *cp++;
        hash *= 0x01000193;
    }
    return hash%NUMMAX;
}

static inline int readn(int fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
        r=(int)read(fd ,bufptr,left);
        if ( r == -1) {
            if (errno == EINTR || errno==EPIPE) continue;
            return -1;
        }
        if (r == 0) return 0;   // gestione chiusura socket
        left    -= r;
        bufptr  += r;
    }
    return (int)size;
}

static inline int writen(int fd, void *buf, size_t size) {
    size_t left = size;
    int r=0;
    char *bufptr = (char*)buf;
    while(left>0) {
        r=(int)write(fd ,bufptr,left);
        if (r == -1) {
            if (errno == EINTR || errno==EPIPE) continue;
            return -1;
        }
        if (r == 0) return 0;
        left    -= r;
        bufptr  += r;
    }
    return 1;
}

#endif /* Header_h */

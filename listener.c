//
//  listener.c
//  Server
//
//  Created by Massimo Puddu on 26/07/17.
//  Copyright © 2017 Massimo Puddu 531379. All rights reserved.
//

#include <listener.h>

int updatemax(fd_set set, int fdmax) {
    for(int i=(fdmax-1);i>=0;--i)
        if (FD_ISSET(i, &set)) return i;
    return -1;
}

void* listener(void *arg){
    int listenfd,notused;
    decision isinset=0;
    ERRORSYSHEANDLER(listenfd, socket(AF_UNIX, SOCK_STREAM, 0),-1, "socket")
    
    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, configurazioni->path[UnixPath], strlen(configurazioni->path[UnixPath])+1);
    
    ERRORSYSHEANDLER(notused, bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)),-1, "bind")
    ERRORSYSHEANDLER(notused, listen(listenfd, SOMAXCONN),-1, "listen")
    
    fd_set set,tmpset;
    // azzero sia il master set che il set temporaneo usato per la select
    FD_ZERO(&set);
    FD_ZERO(&tmpset);
    
    // aggiungo il listener fd al master set
    FD_SET(listenfd, &set);
    // tengo traccia del file descriptor con id piu' grande
    int fdmax=listenfd;
    struct timeval temp;
    opback decisioni;
    while(flag) {
        temp.tv_sec=0;
        temp.tv_usec=2500;
        pthread_mutex_lock(&mtxtoselect);
        while (back != NULL){
            if(back->last==back){
                decisioni=estraib(back);
                back=NULL;
            }
            else decisioni=estraib(back);
            if(decisioni.operazione){
                pthread_mutex_lock(&mtxnconnessioni);
                nconnessioni--;
                pthread_mutex_unlock(&mtxnconnessioni);
                close(decisioni.fd);
            }
            else{ //decisioni.operazione==0){ stava in un else if
                FD_SET(decisioni.fd,&set);
                if(decisioni.fd > fdmax) fdmax = decisioni.fd;
            }
        }
        pthread_mutex_unlock(&mtxtoselect);
        // copio il set nella variabile temporanea per la select
        tmpset = set;
        int notused;
        notused= select(fdmax+1, &tmpset, NULL, NULL, &temp);
        if(notused==-1){
            if(errno==EAGAIN || errno==EINTR) //non gli faccio cercare un fd
                notused=0;
            else if(errno==EBADF) //per togliere il fd lo faccio cerca e chiuderlo
                notused=1;
            errno=0; //EINVAL dificilmente può capitare visto il numero ristretto di connessioni che deve gestire il client
        }
        // cerchiamo di capire da quale fd abbiamo ricevuto una richiesta
        if(notused!=0){
            for(int i=0; i <= fdmax; ++i) {
                if (FD_ISSET(i, &tmpset)) {
                    int connfd;
                    if (i == listenfd) { // e' una nuova richiesta di connessione
                        if((connfd=accept(listenfd, (struct sockaddr*)NULL ,NULL))!=-1){
                            FD_SET(connfd, &set);
                            if(connfd > fdmax) fdmax = connfd;
                            isinset=Connettilo;
                        }
                    }
                    else {
                        connfd = i;  // e' una nuova richiesta da un client già connesso
                        isinset=Exec_op;
                    }
                    if(connfd!=-1){
                        FD_CLR(connfd, &set);
                        if (connfd == fdmax) fdmax = updatemax(set, fdmax);
                        pthread_mutex_lock(&mtxlist);
                        root=inseriscil(root, connfd,NULL,isinset);//se è -2 controllo se non supera il num max connections altrimenti no
                        pthread_cond_signal(&cond);
                        pthread_mutex_unlock(&mtxlist);
                    }
                }
            }
        }
    }
    while (FD_ISSET(fdmax,&set) && fdmax>listenfd) {
        close(fdmax);
        fdmax--;
    }
    pthread_cond_broadcast(&cond);
    close(listenfd);
    return (void*) 0;
}

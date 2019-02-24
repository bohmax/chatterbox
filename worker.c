//
//  worker.c
//  Server
//
//  Created by Massimo Puddu on 02/08/17.
//  Copyright © 2017 Massimo Puddu 531379. All rights reserved.
//

#include <worker.h>

//statistiche
struct statistics chattyStats = { 0,0,0,0,0,0,0 };

void updatestats(unsigned long* stat, int op){
    pthread_mutex_lock(&mtxstat);
    if(op)
        *stat+=1;
    else
        *stat-=1;
    pthread_mutex_unlock(&mtxstat);
}

void setoffline(int conn){
    int err=0,i=0,mutex;
    pthread_mutex_lock(&mtxconn);
    connections=eliminaalbo(connections, conn,&err);
    if(err){
        nutentionline--;
        err=0;
    }
    char* copy=nametodel;
    nametodel=NULL;
    pthread_mutex_unlock(&mtxconn);
    if(copy){
        updatestats(&chattyStats.nonline,0);
        i=(int)get_hash(copy);
        mutex=i/DIV;
        pthread_mutex_lock(&mtxhead[mutex]);
        cercaoff(userstable[i].root, copy);//setta anche offline
        free(copy);
        copy=NULL;
        pthread_mutex_unlock(&mtxhead[mutex]);
    }
}

//reindirizza a un'altro thread l'operazione
void directsend(char* nickname, gestisce spediscimi){
    decision dec=0;
    int dim=spediscimi.gestisci->data.hdr.len;
    message_t* memorizza=NULL;
    ERRORSYSHEANDLER(memorizza,calloc(1,sizeof(message_t)), NULL, "malloc memorizza")
    memorizza->data.buf=NULL;
    if(dim>0){
        ERRORSYSHEANDLER(memorizza->data.buf,malloc(sizeof(char)*dim),NULL,"IMPOSSIBILE TEMP")
        memcpy(memorizza->data.buf, spediscimi.gestisci->data.buf, dim);
    }
    memorizza->data.hdr.len=spediscimi.gestisci->data.hdr.len;
    memorizza->hdr.op=spediscimi.gestisci->hdr.op;
    if(spediscimi.decisione==GroupThread){
        memcpy(memorizza->hdr.sender,nickname,MAX_NAME_LENGTH+1);
        memcpy(memorizza->data.hdr.receiver,spediscimi.gestisci->data.hdr.receiver,MAX_NAME_LENGTH+1);
    }
    else{
        memcpy(memorizza->hdr.sender,spediscimi.gestisci->hdr.sender,MAX_NAME_LENGTH+1);
        memcpy(memorizza->data.hdr.receiver,nickname,MAX_NAME_LENGTH+1);
    }
    if(spediscimi.decisione>SendFileThread) dec=spediscimi.decisione;
    else if(spediscimi.gestisci->hdr.op==POSTTXT_OP) dec=SendMessThreas;
    else dec=SendFileThread;//per file_message
    pthread_mutex_lock(&mtxlist);
    root=inseriscil(root, spediscimi.fd,memorizza,dec);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtxlist);
}

void sendtolist(avl* padre, gestisce spediscimi, int i){
    if(padre==NULL) return;
    sendtolist(padre->left, spediscimi,i);
    sendtolist(padre->right, spediscimi,i);
    if(i)
        directsend(padre->nickname,spediscimi);
    else if(strcmp(padre->nickname,spediscimi.gestisci->hdr.sender)!=0)
        directsend(padre->nickname,spediscimi);
}

void usrlist(gestisce txt,int imtx){
    ERRORSYSHEANDLER(txt.gestisci->data.buf,calloc((nutentionline*(MAX_NAME_LENGTH+1)),sizeof(char)),NULL,"malloc usrlist 1")
    nameo(connections,txt.gestisci->data.buf,0); //tutti gli utenti online in un char
    txt.gestisci->data.hdr.len=nutentionline*(MAX_NAME_LENGTH+1);
    pthread_mutex_lock(&mtxwrite[imtx]);
    if(sendData(txt.fd, &txt.gestisci->data)==-1){ perror("CLIENT DISCONNESSO?"); errno=0;}
    pthread_mutex_unlock(&mtxwrite[imtx]);
    free(txt.gestisci->data.buf);
    txt.gestisci->data.buf=NULL;
    
}

int insertusergroup(char* group, char* sender){
    avl* isonline=NULL;
    isonline=cerca(userstable, sender);
    if(isonline){
        int i=0;
        isonline->group=inseriscialb(isonline->group, group, &i, 0);
        if(i)
            return OP_NICK_ALREADY;
        else
            return OP_OK;
    }
    else return OP_NICK_UNKNOWN;
}

void* pool(void *arg){
    gestisce txt;
    gestisce help;
    message_t* cpyhist=NULL;
    message_data_t mem;//memorizza contenuto file
    char* file=NULL;//path file
    mem.buf=NULL;
    struct stat st;
    size_t nmess=0;
    txt.gestisci=NULL;
    avl*isonline=NULL,*nomepergruppo=NULL;
    online* temp=NULL;
    int i=0,fd, imtx,imtx2=0,modif=1;//modif si usa per indicarmi che name è stato cambiato
    char* name = NULL,* nomeutente=NULL;
    name=malloc(sizeof(char)*(strlen(configurazioni->path[DirName])+strlen("/fileXXXXXX")+1));//per avere la directory di un file temporaneo
    while(flag){
        txt.fd=-1;
        txt.decisione=-1;
        if(modif){sprintf(name, "%s/fileXXXXXX",configurazioni->path[DirName]); modif=0;} //reimposto name in base al val di modif
        i=0;fd=-1;imtx=0;imtx2=0;temp=NULL;nmess=0,file=NULL;
        pthread_mutex_lock(&mtxlist);
        while (root == NULL){
            if(!flag){
                pthread_mutex_unlock(&mtxlist);
                goto LIBERA;//manda alla parte finale del thread per liberare memoria e chiuderlo
            }
            pthread_cond_wait(&cond, &mtxlist);
        }
        if(root->last==root){
            txt=estrai(root);
            root=NULL;
        }
        else txt=estrai(root);
        pthread_mutex_unlock(&mtxlist);
        if(!txt.gestisci){
            ERRORSYSHEANDLER(txt.gestisci, calloc(1,sizeof(message_t)), NULL, "malloc txt.gestisci")
            txt.gestisci->data.buf=NULL;
            if(txt.decisione==Connettilo){
                pthread_mutex_lock(&mtxnconnessioni);
                nconnessioni++;
                if(nconnessioni<configurazioni->dim[MaxConnections]){
                    pthread_mutex_unlock(&mtxnconnessioni);
                    i=OP_OK;
                    i=writen(txt.fd, &i, sizeof(int));
                    if(i==-1){
                        perror("IMPOSSIBILE FAR CONNETTERE");
                        errno=0;
                    }
                }
                else {
                    pthread_mutex_unlock(&mtxnconnessioni);
                    i=OP_FAIL;
                    updatestats(&chattyStats.nerrors, 1);
                    i=writen(txt.fd, &i, sizeof(int));
                    if(i==-1){
                        perror("IMPOSSIBILE FAR DiSCONNETTERE CORRETTAMENTE");
                        errno=0;
                    }
                    goto DISCONNETTI; //lo setto offline
                }
            }
            i=readMsg(txt.fd, txt.gestisci);//uso per mettere i dati su txt.gestisci
            if(i==0 || i==-1){
                if(txt.gestisci)
                    free(txt.gestisci);
                txt.gestisci=NULL;
                errno=0;
            }
        }
        if(txt.gestisci){
            switch(txt.gestisci->hdr.op){
                case REGISTER_OP:{
                    i=(int)get_hash(txt.gestisci->hdr.sender);
                    imtx=i/DIV;
                    pthread_mutex_lock(&mtxhead[imtx]);
                    txt.gestisci->hdr.op=inserisciconindice(userstable, txt.gestisci->hdr.sender, i, 0);
                    if(txt.gestisci->hdr.op==OP_OK)
                        nomeutente=cercalb(userstable[i].root,txt.gestisci->hdr.sender)->nickname;
                    pthread_mutex_unlock(&mtxhead[imtx]);
                    pthread_mutex_lock(&mtxwrite[imtx]);
                    if(sendHeader(txt.fd, &txt.gestisci->hdr)==-1){
                        perror("UTENTE SCOLLEGATO PRIMA DI AOK");
                        errno=0;
                    }
                    pthread_mutex_unlock(&mtxwrite[imtx]);
                    if(txt.gestisci->hdr.op==OP_OK){
                        updatestats(&chattyStats.nusers, 1);
                        txt.decisione=Exec_op;
                        txt.gestisci->hdr.op=CONNECT_OP;
                        txt.gestisci->data.hdr.len=0;
                        if(nomeutente)
                            directsend(nomeutente, txt);
                    } else
                        updatestats(&chattyStats.nerrors, 1);
                    nomeutente=NULL;
                    isonline=NULL;
                }break;
                case CREATEGROUP_OP:{
                    i=(int)get_hash(txt.gestisci->data.hdr.receiver);
                    imtx=i/DIV;
                    fd=(int)get_hash(txt.gestisci->hdr.sender);
                    imtx2=fd/DIV;
                    pthread_mutex_lock(&mtxhead[imtx]);
                    txt.gestisci->hdr.op=inserisciconindice(userstable, txt.gestisci->data.hdr.receiver, i, 1);// con uno lo inserisco come gruppo
                    if(txt.gestisci->hdr.op==OP_OK){
                        isonline=cerca(userstable, txt.gestisci->data.hdr.receiver);
                        i=0;
                        isonline->group=inseriscialb(isonline->group, txt.gestisci->hdr.sender,&i,0);
                        isonline->group->connesso=ITSGROUPOWNER;//da qui so il valore del creatore del gruppo
                    }
                    pthread_mutex_unlock(&mtxhead[imtx]);
                    if(txt.gestisci->hdr.op==OP_OK){
                        pthread_mutex_lock(&mtxhead[imtx2]); //registro nella casella dell' utente il gruppo
                        txt.gestisci->hdr.op=insertusergroup(txt.gestisci->data.hdr.receiver,txt.gestisci->hdr.sender);
                        pthread_mutex_unlock(&mtxhead[imtx2]);
                    }
                    pthread_mutex_lock(&mtxwrite[imtx]);
                    if(sendHeader(txt.fd, &txt.gestisci->hdr)==-1){ perror("UTENTE SCOLLEGATO PRIMA DI AOK"); errno=0;}
                    pthread_mutex_unlock(&mtxwrite[imtx]);
                    if(txt.gestisci->hdr.op==OP_OK)
                        updatestats(&chattyStats.nerrors, 1);
                    isonline=NULL;
                }break;
                case ADDGROUP_OP:{
                    i=(int)get_hash(txt.gestisci->data.hdr.receiver);
                    imtx=i/DIV;
                    fd=(int)get_hash(txt.gestisci->hdr.sender);
                    imtx2=fd/DIV;
                    pthread_mutex_lock(&mtxhead[imtx]);
                    isonline=cerca(userstable, txt.gestisci->data.hdr.receiver);
                    if(isonline && isonline->connesso==ITSGROUP){
                        i=0;
                        isonline->group=inseriscialb(isonline->group, txt.gestisci->hdr.sender, &i, 0);
                        if(i)
                            txt.gestisci->hdr.op=OP_NICK_ALREADY;
                        else
                            txt.gestisci->hdr.op=OP_OK;
                    }
                    else txt.gestisci->hdr.op=OP_FAIL;
                    if(txt.gestisci->hdr.op==OP_OK){
                        pthread_mutex_lock(&mtxhead[imtx2]); //registro nella casella dell' utente il gruppo
                        txt.gestisci->hdr.op=insertusergroup(txt.gestisci->data.hdr.receiver,txt.gestisci->hdr.sender);
                        pthread_mutex_unlock(&mtxhead[imtx2]);
                    }
                    pthread_mutex_unlock(&mtxhead[imtx]);
                    pthread_mutex_lock(&mtxwrite[imtx]);
                    if(sendHeader(txt.fd, &txt.gestisci->hdr)==-1){ perror("UTENTE SCOLLEGATO PRIMA DI AOK"); errno=0;}
                    pthread_mutex_unlock(&mtxwrite[imtx]);
                    if(txt.gestisci->hdr.op!=OP_OK)
                        updatestats(&chattyStats.nerrors, 1);
                    isonline=NULL;
                }break;
                case DELGROUP_OP:{
                    i=(int)get_hash(txt.gestisci->data.hdr.receiver);
                    imtx=i/DIV;
                    fd=(int)get_hash(txt.gestisci->hdr.sender);
                    imtx2=fd/DIV;
                    if(txt.decisione!=GroupThread){
                        pthread_mutex_lock(&mtxhead[imtx]);
                        isonline=cerca(userstable, txt.gestisci->data.hdr.receiver);
                        if(isonline && isonline->connesso==ITSGROUP){
                            nomepergruppo=cercalb(isonline->group, txt.gestisci->hdr.sender);
                            if(nomepergruppo){
                                fd=nomepergruppo->connesso;
                                isonline->group=eliminaalb(isonline->group, txt.gestisci->hdr.sender, &i);//i non viene modificato perchè avrà successo
                                i=txt.decisione;
                                if(fd==-3){// elimina tutti
                                    txt.decisione=GroupThread;
                                    sendtolist(isonline->group, txt,0);
                                    eliminazione(userstable, txt.gestisci->data.hdr.receiver);
                                }
                                txt.decisione=i;
                                if(txt.decisione!=DelUserGroup) txt.gestisci->hdr.op=OP_OK;
                            }
                            else txt.gestisci->hdr.op=OP_NICK_UNKNOWN;
                        }
                    }
                    else txt.gestisci->hdr.op=OP_FAIL;
                    pthread_mutex_unlock(&mtxhead[imtx]);
                    if(txt.gestisci->hdr.op==OP_OK || txt.decisione==GroupThread){
                        pthread_mutex_lock(&mtxhead[imtx2]); //registro nella casella dell' utente il gruppo
                        i=0;
                        isonline=cerca(userstable, txt.gestisci->hdr.sender);
                        isonline->group=eliminaalb(isonline->group, txt.gestisci->data.hdr.receiver, &i);
                        pthread_mutex_unlock(&mtxhead[imtx2]);
                    } else txt.gestisci->hdr.op=OP_FAIL;
                    if(txt.decisione<GroupThread){
                        pthread_mutex_lock(&mtxwrite[imtx]);
                        if(sendHeader(txt.fd, &txt.gestisci->hdr)==-1){ perror("UTENTE SCOLLEGATO PRIMA DI AOK"); errno=0;}
                        pthread_mutex_unlock(&mtxwrite[imtx]);
                    }
                    if(txt.gestisci->hdr.op!=OP_OK){
                        if(txt.decisione!=DelUserGroup)
                            updatestats(&chattyStats.nerrors, 1);
                    }
                    nomepergruppo=NULL;
                    isonline=NULL;
                }break;
                case CONNECT_OP:{
                    i=(int)get_hash(txt.gestisci->hdr.sender);
                    imtx=i/DIV;//variabile che mi da l'indice della mutex
                    pthread_mutex_lock(&mtxhead[imtx]);
                    isonline=cerca(userstable,txt.gestisci->hdr.sender);
                    if(isonline==NULL){
                        txt.gestisci->hdr.op=OP_NICK_UNKNOWN;
                    }
                    else if(isonline->connesso==-1){
                        txt.gestisci->hdr.op=OP_OK;
                        isonline->connesso=txt.fd;
                    }else{
                        txt.gestisci->hdr.op=OP_FAIL;
                    }
                    pthread_mutex_unlock(&mtxhead[imtx]);
                    if(txt.gestisci->hdr.op==OP_OK){
                        updatestats(&chattyStats.nonline, 1);
                        pthread_mutex_lock(&mtxconn);
                        nutentionline++;
                        connections=inseriscialbo(connections, txt.fd,txt.gestisci->hdr.sender);
                        pthread_mutex_unlock(&mtxconn);
                        if(txt.decisione!=SendFileThread){
                            pthread_mutex_lock(&mtxwrite[imtx]);
                            if(sendHeader(txt.fd,&txt.gestisci->hdr)==-1) {perror("l'utente doveva essere online"); errno=0;}
                            pthread_mutex_unlock(&mtxwrite[imtx]);
                        }
                        pthread_mutex_lock(&mtxconn);
                        usrlist(txt,imtx);
                        pthread_mutex_unlock(&mtxconn);
                    } else{
                        updatestats(&chattyStats.nerrors, 1);
                        pthread_mutex_lock(&mtxwrite[imtx]);
                        if(sendHeader(txt.fd,&txt.gestisci->hdr)==-1){ perror("l'utente doveva essere online 1"); errno=0;}
                        pthread_mutex_unlock(&mtxwrite[imtx]);
                    }
                    isonline=NULL;
                }break;
                case USRLIST_OP:{
                    pthread_mutex_lock(&mtxconn);
                    temp=cercalbo(connections, txt.fd);
                    if(temp){
                        ERRORSYSHEANDLER(txt.gestisci->data.buf,calloc((nutentionline*(MAX_NAME_LENGTH+1)),sizeof(char)),NULL,"malloc usrlis")
                        nameo(connections,txt.gestisci->data.buf,0);
                        txt.gestisci->data.hdr.len=nutentionline*(MAX_NAME_LENGTH+1);
                        txt.gestisci->hdr.op=OP_OK;
                    }
                    else{
                        txt.gestisci->hdr.op=OP_FAIL;
                    }
                    pthread_mutex_unlock(&mtxconn);
                    if(txt.gestisci->hdr.op==OP_OK){
                        imtx=(int)get_hash(temp->nickname)/DIV;
                        pthread_mutex_lock(&mtxwrite[imtx]);
                        if((i=sendRequest(txt.fd,txt.gestisci))!=-1){
                            pthread_mutex_unlock(&mtxwrite[imtx]);
                            free(txt.gestisci->data.buf);
                            txt.gestisci->data.buf=NULL;
                        }
                        else {
                            pthread_mutex_unlock(&mtxwrite[imtx]);
                            updatestats(&chattyStats.nerrors, 1);
                            perror("IMPOSSIBILE SPEDIRE hdr DA USRLIST");
                            errno=0;
                        }
                    }
                }break;
                case FILE_MESSAGE:
                case OP_FILETODOWNLOAD:
                case TXT_MESSAGE:
                case OP_SENDFILE:
                case POSTTXT_OP:{
                    imtx2=(int)get_hash(txt.gestisci->hdr.sender)/DIV;
                    i=(int)get_hash(txt.gestisci->data.hdr.receiver);
                    imtx=i/DIV;
                    if(txt.decisione>SendMessThreas || txt.gestisci->data.hdr.len<configurazioni->dim[MaxMsgSize]+1){//se è un file message non controllare la grandezza
                        pthread_mutex_lock(&mtxhead[imtx]);
                        isonline=cercalb(userstable[i].root, txt.gestisci->data.hdr.receiver);//modificare il fatto che sia puntatore e prenderla statica
                        if(!isonline){
                            txt.gestisci->hdr.op=OP_NICK_UNKNOWN;
                            updatestats(&chattyStats.nerrors, 1);
                        }
                        else if(isonline->connesso==-2){
                            nomepergruppo=cercalb(isonline->group, txt.gestisci->hdr.sender);
                            if(nomepergruppo){
                                txt.decisione=DelUserGroup;
                                sendtolist(isonline->group, txt, 1);
                                txt.gestisci->hdr.op=OP_OK;
                                fd=-2;
                                txt.decisione=Exec_op;
                                
                            }
                            else txt.gestisci->hdr.op=OP_NICK_UNKNOWN;
                            pthread_mutex_lock(&mtxwrite[imtx2]);
                            if(sendHeader(txt.fd, &txt.gestisci->hdr)!=1){
                                perror("spedizione messaggio ok");
                                errno=0;
                            }
                            pthread_mutex_unlock(&mtxwrite[imtx2]);
                        }
                        else{
                            if(!isonline->history){
                                ERRORSYSHEANDLER(isonline->history,calloc(configurazioni->dim[MaxHistMsgs],sizeof(message_t)),NULL,"alloc hist")
                                for(int i=0;i<(configurazioni->dim[MaxHistMsgs]-1);i++)
                                    isonline->history[i].data.buf=NULL;
                            }
                            if(txt.gestisci->hdr.op!=OP_SENDFILE && txt.decisione!=GetPrevInThread){
                                nmess=1;//mi fa capire se alla fine dell operazionse se devo liberare memoria oppuere no
                                isonline->nmessaggi_giacenti=(isonline->nmessaggi_giacenti+1)%configurazioni->dim[MaxHistMsgs];
                                if(isonline->history[isonline->nmessaggi_giacenti].data.buf){
                                    free(isonline->history[isonline->nmessaggi_giacenti].data.buf);
                                    isonline->history[isonline->nmessaggi_giacenti].data.buf=NULL;
                                }
                                isonline->history[isonline->nmessaggi_giacenti]=*txt.gestisci;
                                if(txt.decisione<SendMessThreas){
                                    txt.gestisci->hdr.op=OP_OK;
                                    pthread_mutex_lock(&mtxwrite[imtx2]);
                                    if(sendHeader(txt.fd, &txt.gestisci->hdr)!=1){
                                        perror("spedizione messaggio ok");
                                        errno=0;
                                    }
                                    pthread_mutex_unlock(&mtxwrite[imtx2]);
                                    txt.gestisci->hdr.op=isonline->history[isonline->nmessaggi_giacenti].hdr.op;
                                }
                            }
                            if(txt.gestisci->hdr.op==POSTTXT_OP){
                                txt.gestisci->hdr.op=TXT_MESSAGE;
                                updatestats(&chattyStats.nnotdelivered,1);
                            }
                            else if(txt.gestisci->hdr.op==OP_FILETODOWNLOAD){
                                txt.gestisci->hdr.op=FILE_MESSAGE;
                                updatestats(&chattyStats.nfilenotdelivered, 1);
                            }
                            else if(txt.gestisci->hdr.op==OP_SENDFILE)
                                txt.gestisci->hdr.op=OP_OK;
                            fd=isonline->connesso;
                        }
                        pthread_mutex_unlock(&mtxhead[imtx]);
                        if(fd > 0){
                            //devo fare la copia del messaggio prima di salvaro nella tabella utenti
                            pthread_mutex_lock(&mtxwrite[imtx]);
                            if(sendHeader(fd, &txt.gestisci->hdr)==1){
                                if(sendData(fd, &txt.gestisci->data)==1){
                                    pthread_mutex_unlock(&mtxwrite[imtx]);
                                    if(txt.gestisci->hdr.op==TXT_MESSAGE){
                                        updatestats(&chattyStats.ndelivered,1);
                                        updatestats(&chattyStats.nnotdelivered,0);
                                    }
                                    else if(txt.gestisci->hdr.op==OP_OK){
                                        updatestats(&chattyStats.nfiledelivered,1);
                                        updatestats(&chattyStats.nfilenotdelivered,0);
                                    }
                                }
                                else{
                                    pthread_mutex_unlock(&mtxwrite[imtx]);
                                }
                            }
                            else{
                                pthread_mutex_unlock(&mtxwrite[imtx]);
                            }
                        } else if(txt.gestisci->hdr.op==OP_NICK_UNKNOWN && txt.decisione<SendMessThreas){
                            updatestats(&chattyStats.nerrors, 1);
                            pthread_mutex_lock(&mtxwrite[imtx2]);
                            sendHeader(txt.fd, &txt.gestisci->hdr)
                            pthread_mutex_unlock(&mtxwrite[imtx2]);
                        }
                    }
                    else{
                        txt.gestisci->hdr.op=OP_MSG_TOOLONG;
                        updatestats(&chattyStats.nerrors, 1);
                        if(txt.decisione<SendMessThreas){
                            pthread_mutex_lock(&mtxwrite[imtx2]);
                            if(sendHeader(txt.fd, &txt.gestisci->hdr)!=1)
                                errno=0;
                            pthread_mutex_unlock(&mtxwrite[imtx2]);
                        }
                        if(txt.gestisci->data.buf){
                            free(txt.gestisci->data.buf);
                            txt.gestisci->data.buf=NULL;
                        }
                    }
                    if(!nmess){
                        free(txt.gestisci->data.buf);
                        txt.gestisci->data.buf=NULL;
                    }
                    nomepergruppo=NULL;
                    isonline=NULL;
                }break;
                case POSTTXTALL_OP:{
                    i=(int)get_hash(txt.gestisci->hdr.sender);
                    imtx=i/DIV;
                    if(txt.gestisci->data.hdr.len<configurazioni->dim[MaxMsgSize]+1){
                        txt.gestisci->hdr.op=OP_OK;
                        pthread_mutex_lock(&mtxwrite[imtx]);
                        if(sendHeader(txt.fd, &txt.gestisci->hdr)!=1){
                            perror("IN SENDTOALL\n"); errno=0;}
                        pthread_mutex_unlock(&mtxwrite[imtx]);
                        txt.gestisci->hdr.op=POSTTXT_OP;
                        fd=0; //qui fd viene usato come contatore
                        imtx2=0;
                        while (fd<NUMMAX) {
                            pthread_mutex_lock(&mtxhead[imtx2]);
                            while (fd/DIV==imtx2){
                                if(userstable[fd].root && userstable[fd].root->connesso!=-2){
                                    if(i!=fd) sendtolist(userstable[fd].root, txt, 1); //fa spedire separatamente ad altri thread i messaggi per quegli utenti, aggiorno qua le statistiche
                                    else sendtolist(userstable[fd].root, txt, 0);
                                }
                                fd++;
                            }
                            pthread_mutex_unlock(&mtxhead[imtx2]);
                            imtx2=fd/DIV;
                        }
                    }
                    else{
                        txt.gestisci->hdr.op=OP_MSG_TOOLONG;
                        updatestats(&chattyStats.nerrors, 1);
                        pthread_mutex_lock(&mtxwrite[imtx]);
                        if(sendHeader(txt.fd, &txt.gestisci->hdr)!=1){
                            perror("POSTTXTALL 2\n"); errno=0;
                        }
                        pthread_mutex_unlock(&mtxwrite[imtx]);
                    }
                    if(txt.gestisci->data.buf)
                        free(txt.gestisci->data.buf);
                    txt.gestisci->data.buf=NULL;
                }break;
                case POSTFILE_OP:{
                    i=(int)get_hash(txt.gestisci->hdr.sender);
                    imtx2=i/DIV;
                    pthread_mutex_lock(&mtxwrite[imtx2]);
                    if((i=readData(txt.fd, &mem))>0){
                        pthread_mutex_unlock(&mtxwrite[imtx2]);
                        if(mem.hdr.len<(configurazioni->dim[MaxFileSize]*1024)){//se è maggiore dei massimi kb restituisci msgtoolong
                            char* temp=NULL;
                            temp=basename(txt.gestisci->data.buf);
                            ERRORSYSHEANDLER(file,malloc(sizeof(char)*(strlen(configurazioni->path[DirName])+strlen(temp)+2)),NULL,"allocare post") //+2 a causa di terminatore e /
                            i=sprintf(file, "%s/%s",configurazioni->path[DirName],temp);
                            if(i<0){
                                perror("Impossibile sprintf");
                                errno=0;
                            }
                            fd = open(file, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
                            if (fd < 0) {
                                if (errno == EEXIST) {//il file esiste
                                    fd = mkstemp(name);
                                    modif=1;//name è stato modificato
                                    if((i=(int)write(fd, mem.buf, mem.hdr.len))==-1){
                                        perror("impossibile scrivere su file temporaneo");
                                        errno=0;
                                        goto FAIL_FILE;
                                    }
                                    else if((i=chmod(name, 0777))==-1){
                                        perror("impossibile modificare permessi");
                                        errno=0;
                                        goto FAIL_FILE;
                                    }
                                    else if( (i=rename(name, file)) ==-1){
                                        perror("Impossibile rinominare file");
                                        errno=0;
                                        goto FAIL_FILE;
                                    }
                                }
                                else {//non sono riuscito ad aprire il file
                                FAIL_FILE:
                                    updatestats(&chattyStats.nerrors, 1);
                                    txt.gestisci->hdr.op=OP_FAIL;
                                    errno=0;
                                }
                            }
                            else {
                                //il file è stato creato
                                if((i=(int)writen(fd, mem.buf, mem.hdr.len))==-1){
                                    perror("impossibile scrivere su file temporaneo");
                                    errno=0;
                                }
                            }
                            if(txt.gestisci->hdr.op!=OP_FAIL){
                                close(fd);
                                txt.gestisci->hdr.op=OP_OK;
                                pthread_mutex_lock(&mtxwrite[imtx2]);
                                if((sendHeader(txt.fd, &txt.gestisci->hdr))==-1){
                                    perror("IMPOSSIBILE MANDARE FALLIMENTO IN POSTFILE"); errno=0;}
                                pthread_mutex_unlock(&mtxwrite[imtx2]);
                                //spedizione a un altro thread per gestirne l'invio
                                txt.gestisci->hdr.op=OP_FILETODOWNLOAD;
                                char*temp1=NULL;//per non perdere buf
                                temp1=txt.gestisci->data.buf;
                                txt.gestisci->data.buf=temp;
                                txt.gestisci->data.hdr.len=(int)strlen(temp)+1;
                                i=(int)get_hash(txt.gestisci->data.hdr.receiver);
                                imtx=i/DIV;
                                pthread_mutex_lock(&mtxhead[imtx]);
                                isonline=cercalb(userstable[i].root,txt.gestisci->data.hdr.receiver);
                                if(isonline){
                                    if(isonline->connesso==-2)
                                        sendtolist(isonline->group, txt,1);
                                    else
                                        nomeutente=isonline->nickname;
                                }
                                pthread_mutex_unlock(&mtxhead[imtx]);
                                if(nomeutente)
                                    directsend(nomeutente, txt);
                                if(txt.gestisci->data.buf)
                                    free(temp1);
                                temp1=NULL;
                                txt.gestisci->data.buf=NULL;
                                isonline=NULL;
                            } else
                                updatestats(&chattyStats.nerrors, 1);
                        } else{
                            txt.gestisci->hdr.op=OP_MSG_TOOLONG;
                            updatestats(&chattyStats.nerrors, 1);
                        }
                        if(txt.gestisci->hdr.op==OP_FAIL || txt.gestisci->hdr.op==OP_MSG_TOOLONG){//in caso di fallimento preferisco mandare il messaggio di fallimento da qui
                            pthread_mutex_lock(&mtxwrite[imtx2]);
                            if((sendHeader(txt.fd, &txt.gestisci->hdr))==-1){
                                perror("IMPOSSIBILE MANDARE FALLIMENTO IN POSTFILE"); errno=0;}
                            pthread_mutex_unlock(&mtxwrite[imtx2]);
                        }
                        if(file){
                            free(file);
                            file=NULL;
                        }
                    }
                    else{
                        pthread_mutex_unlock(&mtxwrite[imtx2]);
                        perror("Impossibile leggere dal client");
                        errno=0;
                    }
                    if(mem.buf){
                        free(mem.buf);
                        mem.buf=NULL;
                    }
                    if(txt.gestisci->data.buf){
                        free(txt.gestisci->data.buf);
                        txt.gestisci->data.buf=NULL;
                    }
                    nomeutente=NULL;
                }break;
                case GETFILE_OP:{
                    ERRORSYSHEANDLER(file,malloc(sizeof(char)*(strlen(configurazioni->path[DirName])+strlen(txt.gestisci->data.buf)+2)),NULL,"alloc get1")
                    sprintf(file, "%s/%s",configurazioni->path[DirName],txt.gestisci->data.buf);
                    fd = open(file, O_RDONLY , S_IRUSR);
                    if (fd > 0) {
                        if(fstat(fd, &st)!=-1){
                            if(txt.gestisci->data.buf)
                                free(txt.gestisci->data.buf);
                            txt.gestisci->data.buf=NULL;
                            ERRORSYSHEANDLER(txt.gestisci->data.buf,malloc(sizeof(char)*(st.st_size)),NULL,"alloc getfile")
                            if(readn(fd, txt.gestisci->data.buf, (st.st_size))==-1){ txt.gestisci->hdr.op=OP_FAIL; errno=0; }
                            txt.gestisci->data.hdr.len=(unsigned int)st.st_size;
                            close(fd);
                            txt.gestisci->hdr.op=OP_OK;
                        } else {perror("stat fail"); errno=0;}
                    }
                    else{
                        updatestats(&chattyStats.nerrors, 1);
                        txt.gestisci->hdr.op=OP_FAIL;
                    }
                    i=(int)get_hash(txt.gestisci->hdr.sender);
                    imtx=i/DIV;
                    if(txt.gestisci->hdr.op!=OP_FAIL){
                        txt.gestisci->hdr.op=OP_SENDFILE;
                        memcpy(txt.gestisci->data.hdr.receiver,txt.gestisci->hdr.sender,MAX_NAME_LENGTH+1);
                        pthread_mutex_lock(&mtxhead[imtx]);
                        nomeutente=cercalb(userstable[i].root,txt.gestisci->data.hdr.receiver)->nickname;
                        pthread_mutex_unlock(&mtxhead[imtx]);
                        if(nomeutente)
                            directsend(nomeutente, txt);
                        if(txt.gestisci->data.buf){
                            free(txt.gestisci->data.buf);
                            txt.gestisci->data.buf=NULL;
                        }
                    }
                    else{
                        updatestats(&chattyStats.nerrors, 1);
                        i=(int)get_hash(txt.gestisci->hdr.sender);
                        imtx=i/DIV;
                        pthread_mutex_lock(&mtxwrite[imtx]);
                        if(sendHeader(txt.fd, &txt.gestisci->hdr)==-1){
                            perror("get fail error");
                            errno=0;
                        }
                        pthread_mutex_unlock(&mtxwrite[imtx]);
                    }
                    if(file){
                        free(file);
                        file=NULL;
                    }
                    if(txt.gestisci->data.buf){
                        free(txt.gestisci->data.buf);
                        txt.gestisci->data.buf=NULL;
                    }
                    nomeutente=NULL;
                }break;
                case GETPREVMSGS_OP:{
                    i=(int)get_hash(txt.gestisci->hdr.sender);
                    imtx=i/DIV;
                    pthread_mutex_lock(&mtxhead[imtx]);
                    isonline=cercalb(userstable[i].root, txt.gestisci->hdr.sender);
                    if(isonline==NULL){
                        updatestats(&chattyStats.nerrors, 1);
                        txt.gestisci->hdr.op=OP_NICK_UNKNOWN;
                    }
                    else{
                        nomeutente=isonline->nickname;
                        txt.gestisci->hdr.op=OP_OK;
                        if(isonline->nmessaggi_giacenti>-1){
                            if(isonline->history[configurazioni->dim[MaxHistMsgs]-1].data.buf)
                                nmess=configurazioni->dim[MaxHistMsgs];
                            else
                                nmess=(isonline->nmessaggi_giacenti+1);
                            ERRORSYSHEANDLER(cpyhist,calloc(nmess,sizeof(message_t)),NULL,"alloc getprev")
                            imtx2=isonline->nmessaggi_giacenti+1;
                            MODULARE //assegno a fd il valore per l'indice della history
                            for (int x=0; x<nmess; x++){
                                cpyhist[x]=isonline->history[fd];
                                cpyhist[x].data.buf=NULL;
                                ERRORSYSHEANDLER(cpyhist[x].data.buf,malloc(sizeof(char)*isonline->history[fd].data.hdr.len),NULL,"alloc getprev")
                                strncpy(cpyhist[x].data.buf,isonline->history[fd].data.buf,isonline->history[fd].data.hdr.len);
                                fd=(fd+1)%configurazioni->dim[MaxHistMsgs];
                            }
                        } else
                            nmess=0;
                    }
                    pthread_mutex_unlock(&mtxhead[imtx]);
                    if(txt.gestisci->hdr.op==OP_OK){
                        txt.gestisci->data.buf=(char*)&nmess;
                        txt.gestisci->data.hdr.len=sizeof(size_t);
                        pthread_mutex_lock(&mtxwrite[imtx]);
                        if(sendRequest(txt.fd, txt.gestisci)==-1){
                            pthread_mutex_unlock(&mtxwrite[imtx]);
                            perror("NO MSG 1");
                            errno=0;
                        }
                        else{
                            pthread_mutex_unlock(&mtxwrite[imtx]);
                            for(int kevin=0;kevin<nmess;kevin++){
                                help.gestisci=&cpyhist[kevin];
                                help.fd=txt.fd;
                                help.decisione=GetPrevInThread;
                                directsend(nomeutente, help);
                                free(cpyhist[kevin].data.buf);
                                
                            }
                        }
                    }
                    else{
                        updatestats(&chattyStats.nerrors, 1);
                        txt.gestisci->hdr.op=OP_FAIL;
                        pthread_mutex_lock(&mtxwrite[imtx]);
                        if(sendHeader(txt.fd, &txt.gestisci->hdr)==-1){perror("IMPOSSIBILE OP FAIL"); errno=0;}
                        pthread_mutex_unlock(&mtxwrite[imtx]);
                    }
                    isonline=NULL;
                    if(cpyhist)
                        free(cpyhist);
                    cpyhist=NULL;
                    nomeutente=NULL;
                }break;
                case UNREGISTER_OP:{
                    //si può deregistrare solo se stessi
                    if(strcmp(txt.gestisci->hdr.sender,txt.gestisci->data.hdr.receiver)==0){
                        pthread_mutex_lock(&mtxconn);
                        connections=eliminaalbo(connections, txt.fd,&i);
                        if(i){
                            updatestats(&chattyStats.nonline, 0);
                            nutentionline--;
                            i=0;
                        }
                        file=nametodel;
                        nametodel=NULL;
                        i=(int)get_hash(txt.gestisci->hdr.sender);
                        imtx=i/DIV;
                        pthread_mutex_unlock(&mtxconn);
                        if(file){
                            updatestats(&chattyStats.nusers, 0);
                            pthread_mutex_lock(&mtxhead[imtx]);
                            isonline=cerca(userstable, txt.gestisci->hdr.sender);
                            if(isonline->group){
                                txt.decisione=DelUserGroup; //per eviare conflitti in delgroup
                                txt.gestisci->hdr.op=DELGROUP_OP;
                                sendtolist(isonline->group, txt, 1);
                            }
                            userstable[i].root=eliminaalb(userstable[i].root, file, &fd);
                            free(file);
                            file=NULL;
                            pthread_mutex_unlock(&mtxhead[imtx]);
                            txt.gestisci->hdr.op=OP_OK;
                        } else
                            txt.gestisci->hdr.op=OP_FAIL;
                    } else txt.gestisci->hdr.op=OP_FAIL;
                    pthread_mutex_lock(&mtxwrite[imtx]);
                    if(sendHeader(txt.fd, &txt.gestisci->hdr)==-1){ perror("Impossibile spedire deregistrazione");errno=0;}
                    pthread_mutex_unlock(&mtxwrite[imtx]);
                    if(txt.gestisci->hdr.op==OP_FAIL)
                        updatestats(&chattyStats.nerrors, 1);
                    if(txt.gestisci)
                        free(txt.gestisci);
                    txt.gestisci=NULL;
                    isonline=NULL;
                    goto DISCONNETTI;
                }break;
                default:{
                    fprintf(stderr, "operazione non riconosciuta" );
                    txt.gestisci->hdr.op=OP_FAIL;
                    pthread_mutex_lock(&mtxwrite[imtx]);
                    if(sendHeader(txt.fd, &txt.gestisci->hdr)==-1){ perror("Impossibile spedire deregistrazione");errno=0;}
                    pthread_mutex_unlock(&mtxwrite[imtx]);
                    updatestats(&chattyStats.nerrors, 1);
                }break;
            }
            if( txt.decisione<SendMessThreas ){ //elimino le volte in cui faccio rimbalzare le operazioni
                //reinserisco nella select per nuove richieste
                pthread_mutex_lock(&mtxtoselect);
                back=inseriscil(back, txt.fd,NULL,0);
                pthread_mutex_unlock(&mtxtoselect);
            }
        }
        else{
            setoffline(txt.fd);
        DISCONNETTI:pthread_mutex_lock(&mtxtoselect);
            back=inseriscil(back, txt.fd,NULL,1);
            pthread_mutex_unlock(&mtxtoselect);
        }
        if(txt.gestisci)
            free(txt.gestisci);
        txt.gestisci=NULL;
    }
LIBERA: //label per liberare la memoria
    if(txt.gestisci)
        free(txt.gestisci);
    free(name);
    return (void*) 0;
}

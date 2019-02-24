//
//  main.c
//  Server
//
//  Created by Massimo Puddu on 08/06/17.
//  Copyright © 2017 Massimo Puddu 531379. All rights reserved.
//
#ifndef HEADER_H
#include <Header.h>
#endif
#ifndef LISTENER
#include <listener.h>
#endif
#ifndef WORKER
#include <worker.h>
#endif

//thread per la gestione dei segnali e la stampa delle statistiche
void* segnali(void *arg);

//cancella una cartella e tutto il suo sottoalbero
int unlink_file(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);

//pulisce il sistema per far ripartire il server correttamente
void cleanup();

//variabile globale che contiene le informazioni del file configurazioni
static sigset_t   sigpipe_mask;
static sigset_t   sigset_usr;
static char*skt=NULL;
static char*dir=NULL;
struct configuration* configurazioni=NULL;
char* nametodel=NULL;
struct nodi* root=NULL;
struct nodi* back=NULL;
int nutentionline=0;
int nconnessioni=-1;
struct table* userstable=NULL; 
struct on* connections=NULL;
pthread_mutex_t mtxlist=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtxtoselect=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtxhead[NUMMAX/DIV];
pthread_mutex_t mtxwrite[NUMMAX/DIV];
pthread_mutex_t mtxconn=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtxstat=PTHREAD_MUTEX_INITIALIZER;//mutex per le statistiche
pthread_mutex_t mtxnconnessioni=PTHREAD_MUTEX_INITIALIZER;//mutex per il numero di connessioni
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;//condizione per produttore consumatore

volatile sig_atomic_t flag=1;//per chiudere tutti i thread

void* segnali(void *arg){
    //Gestione segnali
    FILE *fp=NULL;
    int ris;
    ERRORSYSHEANDLER(fp,fopen(configurazioni->path[StatFileName], "w"),NULL,"IMPOSSIBILE APRIRE FILE")
    //ERRORSYSHEANDLER(notused,sigaction(SIGUSR1,&s,NULL),-1,"IMPOSSIBILE SEGNALE")
    while(flag){
        if(sigwait(&sigset_usr, &ris)!=0){
                perror("SIGWAIT");
                errno=0;
                exit(0);
        }
        if(ris==SIGUSR1){
            if(printStats(fp)==-1)
                printStats(fp); //se non ristampa non provo a ristamparlo
        }
        else flag=0;
    }
    fclose(fp);
    return (void*) 0;
}

static void usage(const char *progname) {
    fprintf(stderr, "Il server va lanciato con il seguente comando:\n");
    fprintf(stderr, "  %s -f conffile\n", progname);
}

int unlink_file(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);
    
    if (rv)
        perror(fpath);
    
    return rv;
}

void cleanup(){
    unlink(skt);
    nftw(dir, unlink_file, 64, FTW_DEPTH | FTW_PHYS);
    free(skt);
    free(dir);
}

int main(int argc, const char * argv[]) {
    if(argc!=3 || strcmp(argv[1], "-f")){
        usage(argv[0]);
        exit(1);
    }
    int notused;

    configurazioni=getconfig(argv[2]);
    MEMERROR(configurazioni, NULL, ERRMSG)
    ERRORSYSHEANDLER(skt,(char*)malloc(strlen(configurazioni->path[UnixPath])*sizeof(char)+1),NULL,MEMERR)
    ERRORSYSHEANDLER(dir,(char*)malloc(strlen(configurazioni->path[DirName])*sizeof(char)+1),NULL,MEMERR)
    MEMERROR(strncpy(skt, configurazioni->path[UnixPath],strlen(configurazioni->path[UnixPath])+1), NULL, "Impossibile copiare la stringa\n")
    MEMERROR(strncpy(dir, configurazioni->path[DirName],strlen(configurazioni->path[DirName])+1), NULL, "Impossibile copiare la stringa\n")
    userstable=create();
    
    //setto alcuni segnali
    sigset_t saved_mask;
    ERRORSYSHEANDLER(notused,sigemptyset(&sigpipe_mask),-1,"NO SIGEMPY 1")
    ERRORSYSHEANDLER(notused,sigaddset(&sigpipe_mask, SIGPIPE),-1,"NO ADDSET")
    ERRORSYSHEANDLER(notused,sigemptyset( &sigset_usr ),-1,"NO SIGEMPY")
    ERRORSYSHEANDLER(notused,sigaddset( &sigset_usr, SIGUSR1),-1,"NO ADDSET")
    ERRORSYSHEANDLER(notused,sigaddset( &sigset_usr, SIGINT),-1,"NO ADDSET")
    ERRORSYSHEANDLER(notused,sigaddset( &sigset_usr, SIGTERM),-1,"NO ADDSET")
    ERRORSYSHEANDLER(notused,sigaddset( &sigset_usr, SIGQUIT),-1,"NO ADDSET")
    ERRORSYSHEANDLER(notused,pthread_sigmask(SIG_BLOCK, &sigset_usr, NULL),-1,"NO SIGMAS")
    ERRORSYSHEANDLER(notused,pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &saved_mask),-1,"IMPOSSIBILE SEGNALE")
    
    //elimino file vecchia dir
    nftw(configurazioni->path[DirName], unlink_file, 64, FTW_DEPTH | FTW_PHYS);
    mkdir(configurazioni->path[DirName], 0777);
    //Gestione server--------------------
    unlink(skt);
    atexit(cleanup);
    
    //creazione thread
    pthread_t t1[configurazioni->dim[ThreadsInPool]];
    pthread_t listen, segnal;
    //inizializzo la mutex dell'hash
    for(int i=0;i<(NUMMAX/DIV);i++){
        if((notused=pthread_mutex_init(&mtxwrite[i], NULL)<0)){
            perror("impossibile inizializzare mutex");
            exit(errno);
        }
        if((notused=pthread_mutex_init(&mtxhead[i], NULL)<0)){
            perror("impossibile inizializzare mutex");
            exit(errno);
        }
    }
    SYSFREE(notused,pthread_create(&segnal,NULL,&segnali,NULL),0,"thread")
    for(int i=0;i<configurazioni->dim[ThreadsInPool];i++){
        pthread_mutex_init(&mtxhead[i], NULL);
        SYSFREE(notused,pthread_create(&t1[i],NULL,&pool,NULL),0,"thread")
    }
    SYSFREE(notused,pthread_create(&listen,NULL,&listener,NULL),0,"thread")
    SYSFREE(notused,pthread_join(listen,NULL),0,"join")
    SYSFREE(notused,pthread_join(segnal,NULL),0,"join 1")
    for(int i=0;i<configurazioni->dim[ThreadsInPool];i++){
        SYSFREE(notused,pthread_join(t1[i],NULL),0,"join worker")
    }
    //--------------------------------------------------
    freeh(userstable,configurazioni->dim[MaxHistMsgs]-1);
    userstable=NULL;
    freeo(connections);
    connections=NULL;
    if(root)
        freecondl(root);
    if(back)
        freecondl(back);
    liberaconf(configurazioni);
    return 0;
}

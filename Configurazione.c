#include <Configurazione.h>

int comparestr(const void* a, const void* b){
    char **a1= (char **)a;
    char **b1= (char **)b;
    return strncmp(*a1,*b1,4);
}

char* trim(char *str){
    char *inizio=str, *fine;
    while(isspace((unsigned char)inizio[0])){//cerco gli spazi iniziali
        inizio++;
    }
    //elimino gli spazi finali
    fine = inizio + strlen(inizio);
    while((isspace((unsigned char)fine[-1])) && (fine > inizio + 1)){ //fine[-1] indica il carattere prima dell'inizio corrente
        fine--;
    }
    fine[0] = '\0'; //porto str a non avere spazi alla fine della stringa
    if (inizio > str) //sposto a da inizio a fine stringa
        memmove(str, inizio, (fine - inizio) + 1);
    MEMERROR(str, NULL, "Impossibile spostare la memoria");
    if(str[0]!='\0')
        return str;
    return NULL;
}

char *getsubstring(char *tokenizza){
    int i=0;
    while (tokenizza && tokenizza[i]!='=') {
        tokenizza++;
    }
    if(tokenizza[0]=='\0') return NULL;//sarà poi il main a gestire nel caso di null
    tokenizza++;
    trim(tokenizza);
    return tokenizza;
}

config *getconfig(const char *path){
    config *helper=NULL;
    FILE* fileconf;
    int i;
    char* temp,**nomi;
    ERRORSYSHEANDLER(temp,malloc(LENGHT*sizeof(char)),NULL,MEMERR)
    ERRORSYSHEANDLER(nomi,malloc(N*sizeof(char*)),NULL,MEMERR)
    ERRORSYSHEANDLER(fileconf, fopen(path,"r"), NULL, "Errore apertura file 1\n")
    else{
        i=0;
        while(fgets(temp, LENGHT, fileconf) != NULL){ //leggo riga per riga ed escleduo le righe che iniziano con # o isspace che comprende
            if((temp[0]!='#') && (trim(temp))!=NULL){     // ' ' '\n' e altri caratteri simili sia da inizio che fine stringa
                ERRORSYSHEANDLER(nomi[i],malloc((strlen(temp)+1)*sizeof(char)),NULL,MEMERR)
                ERRORSYSHEANDLER(nomi[i], strncpy(nomi[i], temp,strlen(temp)+1), NULL, "Impossibile copiare la stringa\n")
                i++;
            }
        }
    }
    free(temp);
    //se non ho trovato il numero di dati neccessari termina
    if(i!=N){ //se non sono presenti 8 elementi esco
        ERRORSYSHEANDLER(i,fclose(fileconf),EOF,ERRCLOSE)
        return helper;
    }
    qsort(nomi, N, sizeof(char *), comparestr);
    //me li metto in una situazione più comoda
    SWAP(nomi[0], nomi[6])
    //Prendo la sottostringa utile per il progetto, visto l'ordinamento so già in che ordine arriveranno
    ERRORSYSHEANDLER(helper,malloc(sizeof(config)),NULL,MEMERR)
    ERRORSYSHEANDLER(helper->path,malloc(SOTTR*sizeof(char*)),NULL,MEMERR)
    for(i=0;i<SOTTR;i++){
        ERRORSYSHEANDLER(helper->path[i],malloc((strlen(nomi[N-i-1]))*sizeof(char)+1),NULL,MEMERR)
        ERRORSYSHEANDLER(helper->path[i], strncpy(helper->path[i], getsubstring(nomi[N-1-i]), (strlen(nomi[N-i-1])+1)), NULL, "string copy\n")
        free(nomi[N-i-1]);
    }
    char* ptr=NULL; errno=0;
    for (i=0; i<INTERI; i++) {
        if((temp=getsubstring(nomi[i]))==NULL){ //qui gestisco il NULL in modo che atoi non possa causare problemi
            GESTERR
        }
        if(((helper->dim[i]=(int)strtol(temp, &ptr, 10))==0) || nomi[i]==ptr){ //immagino che nessun valore posso essere 0
            GESTERR
        }
        free(nomi[i]);
        nomi[i]=NULL;
    }
    free(nomi);
    nomi=NULL;
    ERRORSYSHEANDLER(i,fclose(fileconf),EOF,ERRCLOSE)
    return helper;
}

void liberaconf(config *lib){
free(lib->path[0]);	
free(lib->path[1]);
free(lib->path[2]);
    free(lib->path);
    lib->path=NULL;
    free(lib);
    lib=NULL;
}

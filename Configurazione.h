//
//  Configurazione.h
//  Server
//
//  Created by Massimo Puddu on 13/06/17.
//  Copyright © 2017 Massimo Puddu 531379. All rights reserved.
//

#ifndef CONFIG
#define CONFIG
#define MEMERR "Impossibile allocare memoria\n"
#ifndef HEADER_H
#include <Header.h>
#endif

#define N  8 //numero elementi del file config
#define INTERI 5 //numero degli elementi int nel conf
#define SOTTR 3
#define LENGHT 512 //lunghezza destinata alle stringhe che memorizzano il file config

#define SWAP(a,b) temp=a; a=b; b=temp; temp=NULL;
#define GESTERR             ERRORSYSHEANDLER(i,fclose(fileconf),EOF,ERRCLOSE) \
free(helper); \
return NULL;
#define ERRMSG "Il file non era del tipo config\n" //messaggio di errore nel caso di dati sbagliati
#define ERRCLOSE "Impossibile chiudere il file di configurazione\n"

/**
 * @file  Configurazione.h
 * @brief Contiene le funzioni e le strutture per memorizzare i dati di configurazione del server
 *
 */

//-------------------------------------------------------------------------------------------------

/**
 *  @struct configuration
 *  @brief struttura che contiene i dati di configurazione
 *
 *  @var path array di stringhe contenente vari path
 *  @var dim contiene interi
 */
typedef struct configuration{
    char **path;
    int dim[INTERI];
}config;
//-------------------------------------------------------------

/**
 funzione ausiliaria della qsort

 @param a primo valore da confrontare
 @param b secondo valore da confrontare

 @return un intero che indica quale valore andrà prima dell'altro
 */
int comparestr(const void* a, const void* b);

/**
 prende la sottostringa da utillizzare dal file di configurazione

 @param tokenizza stringa da tokenizzaee

 @return la stringa tokenizzata
 */
char *getsubstring(char *tokenizza);

/**
 elimina gli spazi bianchi da inizio e fine stringa. Per questa funzione ho preso ispirazione da un utente su stackoverflow e modificato il codice
 @param str stringa da utilizzare

 @return la stringa senza spazi bianchi sia prima dell'inzio che dopo i caratteri finali
 */
char* trim(char *str);

/**
 prende tutte le informazioni dal file config e le inserisce dentro la struttura dati config

 @param path path del file di configurazione

 @return ritorna un config utilizzabile, se fallisce si chiude il server
 */
config *getconfig(const char *path);

/**
 libera la memoria della struttura dati config
 
 @param lib config da liberare
 */
void liberaconf(config *lib);

#endif //CONFIG

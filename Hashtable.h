//
//  Hashtable.h
//  Server
//
//  Created by Massimo Puddu on 14/06/17.
//  Copyright © 2017 Massimo Puddu 531379. All rights reserved.
//
#ifndef HASH
#define HASH
#ifndef HEADER_H
#include <Header.h>
#endif
/**
 * @file  Hashtable.h
 * @brief Contiene le funzioni per il funzionamento della tabella hash
 *
 */

/**
 *  @struct table
 *  @brief tabella hash
 *
 *  @var root un nodo della tabella
 */
typedef struct table{
    struct node* root;
}hash;

/**
 inserimento nell'hash che crea un avl nell'indirizzo hash

 @param table tabella hash in cui inserire il nome
 @param nome  nome da inserire

 @return OP_OK se l'operazione va a buon fine altrimenti ritorna NICKALREADY
 */
int inserisci(hash *table,char*nome);

/**
 inserisce nella tabella hash senza usare la gethash

 @param table tabella hash
 @param name  nome da inserire
 @param i     indica la posizione nella tabella hash
 @param dec   utilizzata per farmi capire se si tratta di un gruppo

 @return OP_OK se l'operazione va a buon fine altrimenti ritorna NICKALREADY
 */
int inserisciconindice(hash *table,char*name,int i, int dec);

/**
 elimina un elemento nell'albero

 @param table tabella hash in cui eliminare l'elemento
 @param nome  nome da eliminare

 @return OP_OK se l'operazione va a buon, altrimenti OP_UNKNOW
 */
int eliminazione(hash *table,char*nome);

/**
 cerca un nodo nella tabella hash

 @param table tabella hash su cui cercare
 @param nome  persona da cercare

 @return ritorna la foglia dell'elemento cercato
 */
struct node *cerca(hash *table,char*nome);

/**
 crea la tabella hash

 @return l'indirizzo alla tabella hash
 */
hash *create();

//libera la tabella hash
/**
 <#Description#>

 @param table tabella hash da eliminare
 @param nmax  numero massimo di nmessaggi giacenti

 @return ritorna NULL
 */
hash *freeh(hash* table, int nmax);

#endif //HASH

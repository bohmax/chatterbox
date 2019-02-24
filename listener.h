//
//  listener.h
//  Server
//
//  Created by Massimo Puddu on 26/07/17.
//  Copyright © 2017 Massimo Puddu 531379. All rights reserved.
//

#ifndef LISTENER
#define LISTENER
#ifndef HEADER_H
#include <Header.h>
#endif
/**
 * @file  listener.h
 * @brief Contiene il thread listener
 *
 */

/**
 aggiorna il valore dell'ultimo file descriptor aperto

 @param set   fd_set da aggiornare
 @param fdmax intero che rappresente l'ultimo file descriptor aperto

 @return il valore del file descriptor più grande
 */
int updatemax(fd_set set, int fdmax);

/**
 thread listener contenente la select

 @param arg non contiene alcun valore

 @return ritona (void*) 0
 */
void* listener(void *arg);

#endif /* listener_h */

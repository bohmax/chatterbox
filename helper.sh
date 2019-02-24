#!/bin/bash
#Scritto da Massimo Puddu matricola 531379
helper="NAME
	helper - elimina file creati n minuti fa o li visualizza

SYNOPSIS
	helper FILE time

DESCRIPTION
	Se si passa come secondo parametro 0 vengono visualizzati tutti gli 
	elementi della DirName, mentre specificando un intero x > 0 si 
	eliminano tutti i file più vecchi di x minuti
	La directory viene estratta dal file che viene passata come 
	primo argomento, la directory deve essere indicata e deve essere 
	unica nel file DirName=directory
AUTHOR
	Written by Massimo Puddu
	"
for nvar in "$@"
do
    if [ "$nvar" = "--help" ];then #Controllo se è presente un --help tra gli argomenti 
		echo "$helper"
		exit 1
	fi
done
	if [ $# -eq 0 ]; then #Controllo se ho passato il numero corretto di elementi
		 echo "$helper"
		 exit 1
	elif ! [ $# -eq 2 ]; then 
		echo "Usage: helper [Dir] integer"
		exit 1
	elif ! [ -f "$1" ]; then #Controllo se il file esiste
		echo "Errore: File non trovato"
		exit 1
	elif ! [ -r "$1" ]; then #Controllo se ho i permessi di lettura
		echo "Errore: Permessi in lettura insufficienti"
		exit 1
	elif ! [ "$2" -gt -1 ] 2>/dev/null; then  #guardo se è un intero, e se è accettabile
    	echo "Errore: Inserire un intero come seconfo argomento >=0"
    	exit 1
	fi
	mem=$( grep DirName "$1" | grep -E '^[^#]+$') #cerco DirName su $1 e elimino dai risultati tutti i dirname che iniziano con cancelletto
	temp=$( echo "$mem" | cut -f 1 -d~) #controllo se inizia con tilde
	if [ "$temp" = "$mem" ];then
		temp=$( echo "$mem" | cut -f 1 -d.) #prendo tutta la parte di stringa prima del path
	fi
	if [ "$temp" = "$mem" ]; then #se non ho trovato la stringa con il punto cerco con slash
		temp=$( echo "$mem" | cut -f 1 -d/)
	fi
	directory=${mem:${#temp}} #seleziono la parte che ha la path con pathern matching
	read -a array <<< $directory
	directory=$(echo ${array[*]/#\~/$HOME}) #nel caso di tilde sostituisco con $HOME
	if [ -z "$directory" ]; then #controllo se ho un risultato
		echo "Errore: Non è stato trovato un DirName"
		exit 1
	elif ! [ -d "$directory" ]; then #controllo se è una directory
		echo "Errore: Su $1 non è presente una directory corretta"
		exit 1
	elif ! [ -r "$directory" ]; then #controllo i permessi di lettura
		echo "Errore: Non abbiamo i permessi per aprire la DirName "
		exit 1
	fi
	if cd "$directory";then
		if [ $2 = 0 ]; then
			list=$(ls | LC_ALL="C" sort) #visualizzo i file nella cartella e li ordine by bytewise
			if [ -z "$list" ]; then
				echo "Non sono presenti file nella directory"
			else echo "$list"
			fi
		elif ! [ -w "$directory" ];then #vedo se posso scrivere e ho tutti i diritti per rimuovere i file
			echo "Errore: Non hai permessi in scritture per rimuovere i file"
			exit 1;
		elif [ "$2" -gt 0 ];then #sono per forza in un caso intero >0
			output=$(find . -mmin +$2 -print -delete) #trovo e cancello i file più vecchi di x minuti
			if [ -z "$output" ]; then
				echo "Non sono stati eliminati file"
			else echo "Sono stati eliminati i seguenti file 
$output"
			fi
		fi
	else echo "Errore: Impossibile aprire la directory"
	fi

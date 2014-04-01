/********************************** 
 * @author      Daniel Valbuena Sosa, adapted from  Johan Hanssen Seferidis
 * @date        07/10/2013
 * Last update: 07/10/2013
 * License:     LGPL
 * 
 *//** @file queue3.h *//*
 ********************************/

/* Library providing a queue where you can add data.  */

/* 
 * Fast reminders:
 * 

 * 
 * */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#include "queue.h"      /* here you can also find the interface to each function */


//static int thpool_keepalive=1;

/* Create mutex variable */
//pthread_mutex_t mutexqueue = PTHREAD_MUTEX_INITIALIZER; /* used to serialize queue access */
pthread_mutex_t mp; // = PTHREAD_MUTEX_INITIALIZER;

/* =================== JOB QUEUE OPERATIONS ===================== */

/* Initialise queue */
dataqueue* dataqueue_init(void){
	dataqueue* dataqueue_p;
	/* Make new queue */
	dataqueue_p = (dataqueue*)malloc(sizeof(dataqueue));
	if (dataqueue_p==NULL){
		fprintf(stderr, "dataqueue_init(): Could not allocate memory for queue\n");
		return NULL;
	}
	/* Initialise semaphore*/
	//dataqueue_p->queueSem = (sem_t*)malloc(sizeof(sem_t));/* MALLOC job queue semaphore */
	//sem_init(dataqueue_p->queueSem, 0, 0); 					/* no shared, initial value */
	pthread_mutex_init(&mp,NULL);

	dataqueue_p->tail = NULL;
	dataqueue_p->head = NULL;
	dataqueue_p->size = 0;
	return dataqueue_p;
}

/* Add data to queue */
void dataqueue_add(dataqueue *dataqueue_p, data_item *newdata_p){ /* remember that job prev and next point to NULL */

	data_item *tailData;
		
	newdata_p->next = NULL;
	newdata_p->prev = NULL;
	
	pthread_mutex_lock(&mp);
	tailData = dataqueue_p->tail;
	if(dataqueue_p->tail == NULL){
		dataqueue_p->tail=newdata_p;
		dataqueue_p->head=newdata_p;
	}
	else{
		dataqueue_p->tail = newdata_p;
		newdata_p->prev = tailData;
		tailData->next = newdata_p;
	}
	
	(dataqueue_p->size)++;
	pthread_mutex_unlock(&mp);
}

/* Remove data from queue */
data_item* dataqueue_remove(dataqueue* dataqueue_p){
	data_item* headData;
	
	pthread_mutex_lock(&mp);
	headData = dataqueue_p->head;
	if(dataqueue_p->size == 0){
	headData = NULL;
	}
	
	else if (dataqueue_p->size == 1){
		dataqueue_p->head = NULL;
		dataqueue_p->tail = NULL;
		(dataqueue_p->size)--;
	}
	
	else{
		dataqueue_p->head = headData->next;
		dataqueue_p->head->prev = NULL;
		(dataqueue_p->size)--;
	}
	pthread_mutex_unlock(&mp);
	
	return headData;
}

/* Get first element from queue */
data_item* dataqueue_peek(dataqueue* dataqueue_p){
	return dataqueue_p->tail;
}

/* Remove and deallocate all data in queue */
void dataqueue_empty(dataqueue* dataqueue_p){
	
	data_item *curdata;
	curdata=dataqueue_p->tail;
	
	while(dataqueue_p->size){
		dataqueue_p->tail=curdata->prev;
		free(curdata);
		curdata=dataqueue_p->tail;
		dataqueue_p->size--;
	}
	
	/* Fix head and tail */
	dataqueue_p->tail=NULL;
	dataqueue_p->head=NULL;
}

/* Print all active queue */
void dataqueue_print(dataqueue* dataqueue_p){

	data_item *curdata;
	curdata = dataqueue_p->head;

	if(curdata != NULL){
		while (curdata->next != NULL){
			printf("valor %s \n",curdata->data);
			curdata = curdata->next;
		} 
	}
	
}

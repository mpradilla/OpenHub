
/********************************** 
 * @author      Daniel Valbuena Sosa, adapted from  Johan Hanssen Seferidis
 * @date        07/10/2013
 * Last update: 07/10/2013
 * License:     LGPL
 * 
 **********************************/

/* Description: Library providing a non lock queue where you can add C3 data on the fly.
 *          
 * 
 *              In this header file a detailed overview of the functions and the queue logical
 *              scheme is present in case tweaking is needed. 
 * */

/* 
 * Fast reminders:
 * 
 * */
                  
/*              ___________________________________________________________        
 *             /                                                           \
 *             |   DATA QUEUE       | data1 | data2 | data3 | data4 | ..   |
 *             \___________________________________________________________/
 * 
 *    Description:       C3 Data is added to queue. Once a thread in the pool
 *                       is idle, it is assigned with the first job from the queue(and
 *                       erased from the queue). It's each thread's job to read from 
 *                       the queue serially(using lock) and executing each job
 *                       until the queue is empty.
 * 
 * 
 *    Scheme:
 * 
___Queue____                      ______ 
|           |       .----------->|_Data0_| Newly added C3 data
|  head------------'             |_Data1_|
|           |                    |_Data2_|
|  tail------------.             |__...__| 
|___________|       '----------->|_size_| C3 Data for thread to take
 * 
 * 
 *    Data0________ 
 *    |           |
 *    |    Int    |
 *    |           |
 *    |           |
 *    |           |         Data1_______ 
 *    |  next-------------->|           |
 *    |___________|         |           |..
 */


#ifndef _QUEUE_

#define _QUEUE_

#include <pthread.h>
#include <semaphore.h>

//thpool_job_t  a data_item
//thpool_jobqueue a job_queue

/* Individual data item */
typedef struct data_item{
	char					data[1024];
	struct data_item*       next;           /**< pointer to next job      */
	struct data_item*       prev;           /**< pointer to previous job  */
}data_item;

typedef struct data_dsm{
	char id[50];			   /* Version SHA Identifier in Database*/
	int cols;		   /*Number of columns of the DSM, also rows number. DSM must be always square*/
	int **dsm;                 /*Doble pointer for DSM data represetation*/
	
	int analysis;
	float result;        

	struct data_dsm* next;     /* pointer to next job */
	struct data_dsm* prev;     /*pointer to previos job*/
}data_dsm;


/* Job queue as doubly linked list */
typedef struct dataqueue{
	data_dsm       *head;                  /**< pointer to head of queue */
	data_dsm       *tail;                  /**< pointer to tail of queue */
	int             size;                  /**< amount of jobs in queue  */
	sem_t           *queueSem;              /**< semaphore(this is probably just holding the same as jobsN) */
}dataqueue;


/* =========================== FUNCTIONS ================================================ */


/* ----------------------- Queue specific --------------------------- */

/**
 * @brief  Initialize queue
 * 
 * Allocates memory for the dataqueue, semaphore and fixes 
 * pointers in dataqueue.
 * 
 * @param  nothing
 * @return queue struct on success,
 *         NULL on error
 */
dataqueue* dataqueue_init(void);

/**
 * @brief Add data to queue
 * 
 * A new data will be added to the queue. The new data MUST be allocated
 * before passed to this function or else other functions like will be broken.
 * 
 * @param pointer to queue
 * @param pointer to the new data(MUST BE ALLOCATED)
 * @return nothing 
 */
//void dataqueue_add(dataqueue *dataqueue_p, data_item *newdata_p);
                   
void dataqueue_add(dataqueue *dataqueue_p, data_dsm *newdata_p);

/**
 * @brief Remove last data from queue. 
 * 
 * This does not free allocated memory so be sure to have peeked() \n
 * before invoking this as else there will result lost memory pointers.
 * 
 * @param  pointer to queue
 * @return 0 on success,
 *         -1 if queue is empty
 */
//data_item* dataqueue_remove(dataqueue *dataqueue_p);

data_dsm* dataqueue_remove(dataqueue *dataqueue_p);
/** 
 * @brief Get tail data in queue (tail)
 * 
 * Gets the tail is inside the queue. This will work even if the queue
 * is empty.
 * 
 * @param  pointer to queue structure
 * @return job a pointer to the tail data in queue,
 *         a pointer to NULL if the queue is empty
 */
//data_item* dataqueue_peek(dataqueue* dataqueue_p);

data_dsm* dataqueue_peek(dataqueue* dataqueue_p);
/**
 * @brief Remove and deallocate all data in queue
 * 
 * This function will deallocate all data in the queue and set the
 * bqueue to its initialization values, thus tail and head pointing
 * to NULL and amount of jobs equal to 0.
 * 
 * @param pointer to dataqueue structure
 * */
void dataqueue_empty(dataqueue* dataqueue_p);

/**
 * @brief Remove and deallocate all data in queue
 * 
 * This function will deallocate all data in the queue and set the
 * bqueue to its initialization values, thus tail and head pointing
 * to NULL and amount of jobs equal to 0.
 * 
 * @param pointer to dataqueue structure
 * */
void dataqueue_print(dataqueue* dataqueue_p);

#endif

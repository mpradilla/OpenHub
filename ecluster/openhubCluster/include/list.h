
/********************************** 
 * @author      Daniel Valbuena Sosa
 * @date        11/10/2013
 * Last update: 11/10/2013
 * License:     LGPL
 * 
 **********************************/


#ifndef _SORTEDLIST_

#define _SORTEDLIST_

#include <pthread.h>
#include <semaphore.h>
#include "udpserver.h"


/* Individual data item */
typedef struct sorted_item{
    double	dataValue;
	double	dataValueSmooth1;
	double	dataValueV;
	double	dataValueX;
	double	timestampValue;
	long 	counterValue;
	struct sorted_item*		next;           /**< pointer to next item      */
	struct sorted_item*		prev;           /**< pointer to next item      */
}sorted_item;


/* Job queue as doubly linked list */
typedef struct sortedlist{
	sorted_item		*head;                  /**< pointer to head of list */
	sorted_item		*tail;					/**< pointer to tail of list */
	int				size;                  /**< size of list  */
	sorted_item		*work_itemS1;				/**< pointer to 8th left item */
	sorted_item		*work_itemV;				/**< pointer to 8th left item */
	sorted_item		*work_itemX;				/**< pointer to 8th left item */
	int				sent;
	
}sortedlist;


/* =========================== FUNCTIONS ================================================ */
sortedlist* sortedlist_init(void);

void datalist_add(sortedlist *c3list, sorted_item *newdata_p);

void free_list(sortedlist *c3list);

void print_list(sortedlist *c3list);

sorted_item* get_list_lastItem(sortedlist *c3list);

void smoothS1_N(int sensor, sortedlist *c3list);

void numerical_Integration(int sensor, sortedlist *c3list);

void numerical_Integration_Cumulative(int sensor, sortedlist *c3list);
#endif
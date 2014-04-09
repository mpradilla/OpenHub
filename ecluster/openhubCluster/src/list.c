#include <stdio.h>
#include <stdlib.h>

#include "unistd.h"
#include "string.h"

#include "list.h"
#include "udpserver.h"
#include "queue.h"

pthread_mutex_t mp_list; // = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mp_list_Smooth_1; // = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mp_list_integration; // = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mp_list_integration_cumulative; // = PTHREAD_MUTEX_INITIALIZER;

/* Initialise list */
sortedlist* sortedlist_init(void){
	sortedlist *c3list;
	/* Make new list */
	c3list = (sortedlist*)malloc(sizeof(sortedlist));
	if (c3list == NULL){
		fprintf(stderr, "sortedlist_init(): Could not allocate memory for list\n");
		return NULL;
	}

	c3list->head = NULL;
	c3list->tail = NULL;
	c3list->work_itemS1 = NULL;
	c3list->work_itemV = NULL;
	c3list->work_itemX = NULL;
	c3list->size = 0;
	c3list->sent = 0;
	pthread_mutex_init(&mp_list,NULL);
	pthread_mutex_init(&mp_list_Smooth_1,NULL);
	
	return c3list;
}

void datalist_add(sortedlist *c3list, sorted_item *newdata_p){
    
	pthread_mutex_lock(&mp_list);
	
    if(c3list->head == NULL){
        c3list->head = newdata_p;
		c3list->tail = newdata_p;
		++c3list->size;
    } 
	else if( newdata_p->timestampValue >= c3list->tail->timestampValue ){
		newdata_p->prev = c3list->tail;
		c3list->tail->next = newdata_p;
		c3list->tail = newdata_p;
		++c3list->size;
	}  

	pthread_mutex_unlock(&mp_list);
}

 
void free_list(sortedlist *c3list) {

	pthread_mutex_lock(&mp_list);
    sorted_item *prev = c3list->head;
    sorted_item *cur = c3list->head;
    while(cur) {
        prev = cur;
        cur = prev->next;
        free(prev);
    }   
	pthread_mutex_unlock(&mp_list);    
}

void print_list(sortedlist *c3list){

    sorted_item *p;
    p = c3list->head;

    while(p){
        printf("VALUE %f - TIMESTAMP %f  \n", p->dataValue,p->timestampValue);
        p = p->next;
    }
}

void smoothS1_N(int sensor, sortedlist *c3list){	
	
	// LOW PASS FILTER + SMOOTH
	if ( c3list->size > 10 ){
		//NO PREVIOUS SMOOTH HAS BEEN DONE OVER ORDERED LIST
		if ( c3list->work_itemS1 != c3list->tail ){
			pthread_mutex_lock(&mp_list_Smooth_1);
			if ( c3list->work_itemS1 == NULL){
				c3list->work_itemS1 = c3list->head;
				c3list->work_itemS1->dataValueSmooth1 = c3list->work_itemS1->dataValue;	
				socketclientC3(sensor,c3list->work_itemS1->counterValue,c3list->work_itemS1->timestampValue,c3list->work_itemS1->dataValueSmooth1);
				//printf("C1 sensor: %d\tvalor: %f\tsmooth: %f\ttime: %f\n",sensor, c3list->work_itemS1->dataValue, c3list->work_itemS1->dataValueSmooth1, c3list->work_itemS1->timestampValue);
				c3list->work_itemS1 = c3list->work_itemS1->next;
				++c3list->sent;				
			}
			//THERE IS (PREVIOUS ELEMENT IS THE HEAD)  OR (NEXT ELEMENT IS THE TAIL) IN THE ORDERED LIST
			else if ( (c3list->work_itemS1->prev == c3list->head) || (c3list->work_itemS1->next == c3list->tail) ){
				c3list->work_itemS1->dataValueSmooth1 = (c3list->work_itemS1->prev->dataValue + c3list->work_itemS1->dataValue*2 + c3list->work_itemS1->next->dataValue)/4 ;
				//printf("s%f  - p%f - v%f - n%f\n", c3list->work_itemS1->dataValueSmooth1, c3list->work_itemS1->prev->dataValue, c3list->work_itemS1->dataValue, c3list->work_itemS1->next->dataValue );
				socketclientC3(sensor,c3list->work_itemS1->counterValue,c3list->work_itemS1->timestampValue,c3list->work_itemS1->dataValueSmooth1);
				//printf("C2 sensor:%d \t valor:%f \t smooth: %f \t time: %f \n",sensor, c3list->work_itemS1->dataValue, c3list->work_itemS1->dataValueSmooth1, c3list->work_itemS1->timestampValue);
				c3list->work_itemS1 = c3list->work_itemS1->next;
				++c3list->sent;	
				}
			//THERE ARE TWO PREVIOUS ELEMENTS IN THE ORDERED LIST
			else if ( (c3list->work_itemS1->prev->prev == c3list->head) || (c3list->work_itemS1->next->next == c3list->tail) ){
				c3list->work_itemS1->dataValueSmooth1 = (c3list->work_itemS1->prev->prev->dataValue + c3list->work_itemS1->prev->dataValue*2 + c3list->work_itemS1->dataValue*3 + c3list->work_itemS1->next->dataValue*2 + c3list->work_itemS1->next->next->dataValue)/9;
				socketclientC3(sensor,c3list->work_itemS1->counterValue,c3list->work_itemS1->timestampValue,c3list->work_itemS1->dataValueSmooth1);
				//printf("C3 sensor: %d\tvalor: %f\tsmooth: %f\ttime: %f\n",sensor, c3list->work_itemS1->dataValue, c3list->work_itemS1->dataValueSmooth1, c3list->work_itemS1->timestampValue);
				c3list->work_itemS1 = c3list->work_itemS1->next;
				++c3list->sent;	
			}
			//THERE ARE 3 OR MORE ELEMENTS IN THE ORDERED LIST
			else{
				c3list->work_itemS1->dataValueSmooth1 = (c3list->work_itemS1->prev->prev->prev->dataValue + c3list->work_itemS1->prev->prev->dataValue*2 + c3list->work_itemS1->prev->dataValue*3 + c3list->work_itemS1->dataValue*4 + c3list->work_itemS1->next->dataValue*3 + c3list->work_itemS1->next->next->dataValue*2 + c3list->work_itemS1->next->next->next->dataValue)/16;
				socketclientC3(sensor,c3list->work_itemS1->counterValue,c3list->work_itemS1->timestampValue,c3list->work_itemS1->dataValueSmooth1);
				//printf("C4 sensor: %d\tvalor: %f\tsmooth: %f\ttime: %f\n",sensor, c3list->work_itemS1->dataValue, c3list->work_itemS1->dataValueSmooth1, c3list->work_itemS1->timestampValue);
				c3list->work_itemS1 = c3list->work_itemS1->next;
				++c3list->sent;	
			}

			//printf("sensor %d * smooth %f * valor %f * time %f\n", sensor, work_item->dataValueSmooth1,work_item->dataValue, work_item->timestampValue);
		pthread_mutex_unlock(&mp_list_Smooth_1);
		}
	}
}

void numerical_Integration(int sensor, sortedlist *c3list){
	pthread_mutex_lock(&mp_list_integration);
	// NON CUMULATIVE INTEGRATION	
	if ( c3list->size > 0 && (c3list->work_itemS1 != NULL) ){
		//FIRST ELEMENT SHOULD BE ZERO
		if ( c3list->work_itemV != c3list->work_itemS1 ){
			if ( c3list->work_itemV == NULL){
				c3list->work_itemV = c3list->head;
				c3list->work_itemV->dataValueV = 0;
				socketclientC3(sensor, c3list->work_itemV->counterValue, c3list->work_itemV->timestampValue, c3list->work_itemV->dataValueV);
				//printf("Vel Sensor: %d\tvalor: %f\tVelocity: %f\ttime: %f\n",sensor, c3list->work_itemV->dataValue, c3list->work_itemV->dataValueSmooth1, c3list->work_itemV->timestampValue);
							
			}
			//THERE IS AT LEAST A PREVIOUS ELEMENT INTEGRATED IN THE LIST
			else if ( c3list->work_itemV->next != NULL ){
				c3list->work_itemV->dataValueV = (c3list->work_itemV->timestampValue - c3list->work_itemV->prev->timestampValue) * ((c3list->work_itemV->dataValueSmooth1 + c3list->work_itemV->prev->dataValueSmooth1)/2);
				}
			socketclientC3(sensor, c3list->work_itemV->counterValue, c3list->work_itemV->timestampValue, c3list->work_itemV->dataValueV);
			//printf("Vel Sensor: %d\tvalor: %f\tVelocity: %f\ttime: %f\n",sensor, c3list->work_itemV->dataValueSmooth1, c3list->work_itemV->dataValueV, c3list->work_itemV->timestampValue);
			c3list->work_itemV = c3list->work_itemV->next;
			++c3list->sent;
		}
	}
	pthread_mutex_unlock(&mp_list_integration);
}

void numerical_Integration_Cumulative(int sensor, sortedlist *c3list){
	pthread_mutex_lock(&mp_list_integration_cumulative);
	
	// NON CUMULATIVE INTEGRATION	
	if ( c3list->size > 0 && (c3list->work_itemV != NULL) ){
		//FIRST ELEMENT SHOULD BE ZERO
		if ( c3list->work_itemX != c3list->work_itemV ){
			if ( c3list->work_itemX == NULL){
				c3list->work_itemX = c3list->head;
				c3list->work_itemX->dataValueX = 0;
				socketclientC3(sensor, c3list->work_itemX->counterValue, c3list->work_itemX->timestampValue, c3list->work_itemX->dataValueX);
				//printf("Pos Sensor: %d\tvalor: %f\tPosition: %f\ttime: %f\n",sensor, c3list->work_itemX->dataValueSmooth1, c3list->work_itemX->dataValueX, c3list->work_itemX->timestampValue);
							
			}
			//THERE IS AT LEAST A PREVIOUS ELEMENT INTEGRATED IN THE LIST
			else if ( c3list->work_itemX->next != NULL ){
				c3list->work_itemX->dataValueX = c3list->work_itemX->prev->dataValueX + (c3list->work_itemX->timestampValue - c3list->work_itemX->prev->timestampValue) * ((c3list->work_itemX->dataValueV + c3list->work_itemX->prev->dataValueV)/2);
				}
			socketclientC3(sensor, c3list->work_itemX->counterValue, c3list->work_itemX->timestampValue, c3list->work_itemX->dataValueX);
			//printf("Pos Sensor: %d\tvalor: %f\tPosition: %f\ttime: %f\n",sensor, c3list->work_itemX->dataValueSmooth1, c3list->work_itemX->dataValueX, c3list->work_itemX->timestampValue);
			c3list->work_itemX = c3list->work_itemX->next;
			++c3list->sent;
		}
	}
	pthread_mutex_unlock(&mp_list_integration_cumulative);
}
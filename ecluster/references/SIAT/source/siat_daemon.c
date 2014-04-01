#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mpi.h"
#include <unistd.h>  
#include <time.h>  

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define WORKTAG 1
#define BUFFER_SIZE 1024

#include "thpool.h"
#include "queue.h"
#include "udpserver.h"
#include "list.h"
#include "siat_daemon.h"

/* PROTOTYPE OF RECEIVE UDP SOCKET MESSAGE TASK */
void *taskIreceive(void *arg);

/* PROTOTYPE OF RECEIVE MPI MESSAGE TASK */
void *task_receive_MPI(void *arg);

/* PROTOTYPE OF PRINT QUEUE TASK */
void *taskPrint(void *arg);

/* PROTOTYPE OF MPI SEND MESSAGE TASK */
void *task_send_MPI(void *arg);

/* PROTOYPE ANALYSE AND SEND PROCESSED DATA */
void *analyze_data(void *arg);

int comm_rank, comm_size, thread_level_provided;

/*  MAIN FUNCTION  */
int main(int argc, char **argv){

	/* MPI VARIABLES TO HANDLE PROCESSES ERROR */
	int mpi_err;

	/* MPI INITIALIZE PROCESSES WITH MULTIPLE THREADS SUPPORT */
    mpi_err = MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE,&thread_level_provided);
	
	/* HANDLE MPI ERROR */
    if (mpi_err != MPI_SUCCESS){
		printf("MPI_Init_thread failed with mpi_err=%d\n", mpi_err);
		exit(-1);
    }
    MPI_Comm_rank(MPI_COMM_WORLD,&comm_rank);	/* ID PROCESS IDENTIFIER */
    MPI_Comm_size(MPI_COMM_WORLD,&comm_size);	/* GET PROCESSES SIZE */
	
	/* CHECK THAT ONLY RUN WITH EXACTLY 6 PROCESSES */
	if (comm_size != 5) {
		if (comm_rank == 0) {
			printf("**** MUST EXECUTE WITH EXACTLY 2 PROCESSES ****\n");
		}
		MPI_Finalize();	       /* Quit if there is only one process */
		return 0;
	}

	/* MASTER DAEMON PROCESS */
	if( comm_rank == 0 ){

		/* CREATE THREAD TO RECEIVE DATA */
		pthread_t  Thread_receive;
		/* PRINT QUEUE */
		//pthread_t  Thread_printqueue;
		/* CREATE THREAD POOL TO SEND DATA */
		thpool_t* threadpool_Isend;
		/* CREATE QUEUE TO HANDLE INCOMING DATA */
		dataqueue* c3queue;

		/* INITIALIZE QUEUE */
		c3queue = dataqueue_init();

		/* THREAD PARA RECIBIR Y GUARDAR LOS DATOS */
		pthread_create(&Thread_receive,(void*)NULL, (void *) taskIreceive, (void *)c3queue);
		
		/* THREAD PARA ENVÍAR LA INFORMACIÓN */
		threadpool_Isend=thpool_init(4);    
		/*  ESTE WHILE SE EJECUTA MIENTRAS EXISTA ELEMENTOS EN LA COLA */
		while (1){
			if (c3queue->size > 0){
				thpool_add_work(threadpool_Isend, (void *) task_send_MPI, (void *)c3queue);
			}
		}
		pthread_join(Thread_receive, NULL);
	}
	
	/* ACCELERATION X ANALYSIS PROCESS  DIMENSION: (m/s^2) */
	else{
		/* CREATE THREAD TO RECEIVE DATA */
		pthread_t  Thread_receive_R1;
		/* CREATE THREAD POOL TO ANALYZE DATA */
		thpool_t* threadpool_Analyze;
		/* CREATE QUEUE TO HANDLE INCOMING DATA */
		dataqueue* c3queue_R1;
		/* CREATE SORTED LIST TO PROCESS DATA */
		sortedlist* c3list_R1;
		/* INITIALIZE QUEUE */
		c3queue_R1 = dataqueue_init();
		
		/* INITIALIZE SORTED LIST */
		c3list_R1 = sortedlist_init();
		/* INITIALIZE THREAD POOL */
		threadpool_Analyze = thpool_init(4);
		int sizelist = 0;
		
		//pthread_mutex_init(&mp_smooth,NULL);
		
		// PARAMETERS DEFINITION
		parameters_item* params;
		params = (parameters_item*)malloc(sizeof(parameters_item));
		params->queue = c3queue_R1;
		params->list = c3list_R1;	
		params->sensor = comm_rank;	
		
		/* THREAD PARA RECIBIR Y GUARDAR LOS DATOS */
		pthread_create(&Thread_receive_R1,(void*)NULL, (void *) task_receive_MPI, (void *)params);

	
		/*  THREAD POOL ADDS JOBS ALWAYS */
		while (1){
			if ( sizelist <= c3list_R1->size ){
				thpool_add_work(threadpool_Analyze, (void *) analyze_data, (void *)params);
				++sizelist;
			}
		}
		pthread_join(Thread_receive_R1, NULL);
	}
	
	mpi_err = MPI_Finalize();
	return 0;
}

/* RECEIVE MESSAGE TASK   ABRE EL SOCKET SERVER */
void *taskIreceive(void *arg){
	dataqueue *dataqueue_p = ((dataqueue *)arg);
	socketpua(dataqueue_p);
	return(NULL);
}

/* PRINT QUEUE TASK */
void *taskPrint(void *arg){
	dataqueue *dataqueue_p = ((dataqueue *)arg);
	data_item *newdata_p = NULL;

	while (1){
		newdata_p = dataqueue_remove(dataqueue_p);
		if (newdata_p != NULL){
		printf("tamaño del queue %d  y el valor %s \n",dataqueue_p->size,newdata_p->data);
		}
		//dataqueue_print(dataqueue_p);
	}
	return(NULL);
}

/* SEND MESSAGE TASK */
void *task_send_MPI(void *arg){
	char data_send[1024];
	MPI_Request request1;
	MPI_Status status1;
	MPI_Request request2;
	MPI_Status status2;
	MPI_Request request3;
	MPI_Status status3;
	MPI_Request request4;
	MPI_Status status4;
	data_item* pulledData;
	
	dataqueue *dataqueue_p = ((dataqueue *)arg);
	pulledData = dataqueue_remove(dataqueue_p);
	
	if (pulledData != NULL){
		strcpy(data_send, pulledData->data);
		free(pulledData);
		MPI_Isend(data_send, 1024,MPI_CHAR, 1, 0, MPI_COMM_WORLD, &request1);
		MPI_Isend(data_send, 1024,MPI_CHAR, 2, 0, MPI_COMM_WORLD, &request2);
		MPI_Isend(data_send, 1024,MPI_CHAR, 3, 0, MPI_COMM_WORLD, &request3);
		MPI_Isend(data_send, 1024,MPI_CHAR, 4, 0, MPI_COMM_WORLD, &request4);
		MPI_Wait (&request3, &status3);
		MPI_Wait (&request2, &status2);
		MPI_Wait (&request1, &status1);
		MPI_Wait (&request4, &status4);
	}
	return(NULL);
}

/* RECEIVE_SPECIFIC_ANALYSYS MESSAGE TASK */
void *task_receive_MPI(void *arg){
	char recv_dataR[1024];
	MPI_Request request;
	MPI_Status status;
	parameters_item *paramsR1 = ((parameters_item *)arg);
	sortedlist *datalist_R = paramsR1->list;

	int valTokenPosition;
	char *charActiveToken;
	double timestamp;
	int counter;
	sorted_item *newdata_item;
	
	while(1){
		MPI_Irecv(recv_dataR, 1024, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
		MPI_Wait (&request, &status);
		if(strlen(recv_dataR) > 0){
			
			valTokenPosition = 0;
			charActiveToken = strtok(recv_dataR, "&:");
			while (charActiveToken != NULL && valTokenPosition <= comm_rank + 2){
				//CATCH COUNTER
				if(valTokenPosition == 1){
					counter = atoi(charActiveToken);
				}
				//CATCH TIMESTAMP
				if(valTokenPosition == 2){
					timestamp = (double)atof(charActiveToken);
				}
				else if(valTokenPosition == comm_rank + 2){
					// CREATE NEW DATA ITEM AND ADD IT INTO A SORTED LIST  SOBRA UN MALLOC
					newdata_item = (sorted_item *)malloc(sizeof(sorted_item));
					newdata_item->dataValue = (double)atof(charActiveToken);
					newdata_item->dataValueSmooth1 = 0.0;
					newdata_item->dataValueV = 0.0;
					newdata_item->dataValueX = 0.0;
					newdata_item->timestampValue = timestamp;
					newdata_item->counterValue = counter;
					newdata_item->next = NULL;
					newdata_item->prev = NULL;
					
					//NEW DATA_ITEM IS ADDED TO SORTED LIST STRUCTd
					datalist_add(datalist_R, newdata_item);
				}
				charActiveToken = strtok(NULL, "&:");
				valTokenPosition ++;
			}	
		}
	}
	return(NULL);
}

void *analyze_data(void *arg){
	parameters_item *paramsR1 = ((parameters_item *)arg);
	sortedlist *datalist_R = paramsR1->list;
	int	sensor = paramsR1->sensor;

	if ( sensor == 1 || sensor == 2 || sensor == 3 ){
		smoothS1_N(sensor, datalist_R);
		numerical_Integration(sensor + 3, datalist_R);
		numerical_Integration_Cumulative(sensor + 6, datalist_R);
	}

return(NULL);
}

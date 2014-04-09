#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mpi.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
//#include "../include/utf8.h"
#include "../include/queue.h"
#include "../include/matrix.h"

#define Matrix_create


void *taskIreceive(void *arg);

int comm_rank, comm_size, thread_level_provided;


struct matrix{

	int id;
	int cols;
	int **dsm;
	//Matrix *dsm;

};



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
	
    char procname[MPI_MAX_PROCESSOR_NAME];
    int lenname;
	

    /* MASTER DAEMON PROCESS */
    if( comm_rank == 0 ){
	// Get the name of the processor
	MPI_Get_processor_name(procname, &lenname);
 
	// Print Hello from master process
	printf("Hello world from master process %s, rank %d out of %d processors\n",procname, comm_rank, comm_size);
        
        //Create Thread to receive data to analyze
	pthread_t Thread_receive;
        
	//Create Queue to handle incoming data
	dataqueue* workQueue;	

	pthread_create(&Thread_receive, (void*)NULL, (void *) taskIreceive, (void *) workQueue);	

        while(1)
	{
		printf(".");
		sleep(1); 
	}

    }
	
	/* SLAVE PROCESS */
	else{
		// Get the name of the processor
		MPI_Get_processor_name(procname, &lenname);
 		
		//pthread_t Thread_receive;
		//pthread_create(&Thread_receive, (void*)NULL, (void*) taskIreceive, (void *) NULL);		

		// PHello from master process
		printf("Hello world from slave process %s, rank %d out of %d processors\n",procname, comm_rank, comm_size);

		//while(1)
		//{
		//	printf("waiting connections in process %s ... \n", procname);
		//	sleep(1);
		//}
	}
	
	mpi_err = MPI_Finalize();
	return 0;
}

void *taskIreceive(void *arg){

    printf("Thread to receive data initializing...\n");

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    char sendBuff[4096] = {0};
    time_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '\0', sizeof(serv_addr));
    memset(sendBuff, '\0', sizeof(sendBuff));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);	
    serv_addr.sin_port = htons(5001);
 
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    listen(listenfd,10);
	
    int n= 0, k;
    char recvBuff[4096];
    char *saveID, *saveCols, *saveDSM;	
    int count = 0;
    char copy[100];

    while(1)
    {
	printf("SOCKET INITIALIZED! listening to connections...\n");
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

	n = recv(connfd,&recvBuff,4096,0);
        printf("Rec: %d bytes \n", n);
	
	char *start, *end;
	char *pch;

	pch=malloc(sizeof(char)*strlen(recvBuff));
	printf("size of recvBuff:%d\n",strlen(recvBuff));
	pch = strtok(recvBuff, ":");
	while(pch != NULL)
	{
	    if(count==0)
	    {
	        start= malloc(sizeof(char)*strlen(pch));
		strcpy(start,pch);
		//start=pch;
	    }
	    else if(count==1)
	    {
		saveID=malloc(sizeof(int));
	    	//saveID=pch;
		strcpy(saveID,pch);
	    }
            else if(count==2)
	    {	
		printf("size of cols num:%d\n",strlen(pch));
		saveCols = malloc(sizeof(char)*strlen(pch));
		//saveCols= pch;
		strcpy(saveCols,pch);
	    }	
	    else if(count==3)
	    {
		saveDSM = malloc(sizeof(char)*strlen(pch));
		strcpy(saveDSM,pch);
		//saveDSM=pch;
	    }
	    else if (count==4)
	    {
		end= malloc(sizeof(char)*strlen(pch));
		strcpy(end,pch);
		//end=pch;
	    }
	    count++;
	    pch =strtok(NULL, ":");
	}

        count=0;
	printf("%s - %s \n",start, end);
	printf("checking message integrity...\n");
	if(strcmp(start,"$")==0 && strcmp(end,"$")==0){	
		printf("MESSAGE SUCCESSFUL RECEIVED \n");		
	}

	printf("START :%s\n", start);
	printf("%s\n", saveID );
	printf("%s\n", saveCols);
        printf("%s\n", saveDSM);
	printf("The lenght from dsm is: %i\n",strlen(saveDSM)); 
		
	int i=0, j=0;
	struct matrix dsm;
	
	dsm.id = atoi(saveID);
	dsm.cols = atoi(saveCols);
        

	printf("Free variables...\n");
	free(saveID);
	printf("1\n");
	free(saveCols);
	printf("2\n");
	free(end);
	printf("3\n");
	free(start);
	printf("4\n");
	free(pch);
	printf("ok\n");

 	//int **hh;	
	//int mm[dsm.cols][dsm.cols]; 
        	
	//int **matrix= NULL;
	int rownum;

	dsm.dsm  = malloc(sizeof(int*)*dsm.cols);
	for(rownum=0; rownum<dsm.cols; rownum++)
	{
	  //  matrix[rownum] = NULL;
	    dsm.dsm[rownum] = malloc(sizeof(int)*dsm.cols);
	    //matrix[rownum] = NULL;
	}
 	printf("Memory Allocated\n");	

	//dsm.dsm[0][1]=0;
	//matrix=NULL;
	
	//dsm.dsm = malloc(dsm.cols*sizeof(int*));
	//for(i=0;i<dsm.cols;i++){
	//     dsm.dsm[i] = malloc(dsm.cols*sizeof(int));
	//}
	//dsm = (matrix**)realloc(dsm, saveCols* sizeof(matrix));
	int row=0;
	int col=0;
	int iter=0;
	int toInsert=0;
	char *toConv;
	toConv = malloc(sizeof(char));
	//int rr=0;
	//int cc=0;
	for(iter=0; saveDSM[iter]!='\0';iter++)
	{	
	    printf("%c",saveDSM[iter]);
	    if (saveDSM[iter]==',')
	    {
		col++;
	    }
	    else if (saveDSM[iter]==';')
	    {
		col=0;
		row++;
	    } 	
	    else
	    {
	//	rr+=1;
 	    	//mm[row][col]=saveDSM[iter];
		if(row<dsm.cols-1 && col<dsm.cols-1)
		{
	//	   cc+=1;
		   *toConv =saveDSM[iter];
		   toInsert= atoi(toConv);
		
		   //printf("%d", toInsert);
		//   printf("%d-",toInsert);
		 
		 dsm.dsm[row][col]=toInsert;
		} 
	    }
		//matrix[row][col] =1;
		//printf(".%d",saveDSM[iter]);	
	}
	//printf("rr: %d  cc: %d \n", rr,cc);
	printf("Free dsm and conv...\n");
	free(toConv);
	toConv=NULL;
	free(saveDSM);
	saveDSM=NULL;
	//memset(saveDSM,0,sizeof(saveDSM));
	printf("ok\n");

	printf(" convert...\n");
	
        for(i=0; i<dsm.cols-1;i++)
	{
	    for(j=0;j<dsm.cols-1;j++)
	    {
		//matrix[i][j]= mm[i][j];
		printf("%d,", dsm.dsm[i][j] );	
	    }
	    printf(";");
	}
	

	//matrix.dsm = (char*) malloc((sizeof(recvBuff) +1 )* sizeof(char));
	//strcpy(matrix.dsm, recvBuff);


	//create data for queue
	//data_q = ()


	//ADD DSM TO QUEUE
	//dataqueue_add(workQueue, mm
     
	printf("\nFree memory...\n");
        printf("dsm cols: %d\n",dsm.cols);	
	    int ifree;	
	    for(ifree=0; ifree<dsm.cols-1;ifree++)
	    {
		//printf("_\n");
	        free(dsm.dsm[ifree]);	
		printf("ifree:%d\n",ifree);
	    }
	printf("Inner pointer freed\n");
	//dsm.dsm='\0';	 
	free(dsm.dsm);
	dsm.dsm = NULL;
	
	//matrix=NULL;
	//matrix=0;
	printf("ok\n");

		
//	free(dsm);	
	printf("\n");   
        ticks = time(NULL);
	snprintf(sendBuff, sizeof(sendBuff), "HELLO from Cluster", ctime(&ticks));     
	//write(connfd, sendBuff, strlen(sendBuff));
	write(connfd, "Hello from cluster", strlen("Hello from cluster"));	
	//recvBuff = {0};

	printf("CONNECTION RECEIVED \n");	
	memset(recvBuff,'\0',sizeof(recvBuff));
	memset(sendBuff,'\0',sizeof(sendBuff));

	close(connfd);
	sleep(1);

    }

}


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

void *taskIreceive(void *arg);

int comm_rank, comm_size, thread_level_provided;


struct repo{

	int id;
	int cols;
	//char* dsm;
	int dsm[][];

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
        pthread_create(&Thread_receive, (void*)NULL, (void *) taskIreceive, (void *) NULL);	

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

    char sendBuff[1025] = {0};
    time_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);	
    serv_addr.sin_port = htons(5001);
 
    //printf("socke addr %s", serv_addr);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    listen(listenfd,10);
	
    int n= 0, k;
    char recvBuff[1024];
    char *saveID, *saveCols, *saveDSM;	
    char *token, *str1;
    int count = 0;
    char copy[100];

    //const uint8_t * str =0;
    while(1)
    {
	printf("SOCKET INITIALIZED! listening to connections...\n");
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
	//printf("IP: %s", inet_ntoa(client_addr.sin_addr));

	n = recv(connfd,&recvBuff,1024,0);
        printf("Rec: %d bytes \n", n);
	//sprintf(recvBuff,"%s",(char*)str);
	
	char *start, *end;
	char *pch;
	pch = strtok(recvBuff, ":");
	while(pch != NULL)
	{
	    //printf("%s\n", pch);
	    if(count==0)
		start=pch;
	    else if(count==1)
	    	saveID=pch;
            else if(count==2)
		saveCols= pch;
	    else if(count==3)
		saveDSM=pch;
	    else if (count==4)
		end=pch;
	    
	    count++;
	    pch =strtok(NULL, ":");
	
	}
        count=0;
	printf("%s\n",recvBuff);
	
	if(strcmp(start,"$")==0 && strcmp(end,"$")==0){	
		printf("MESSAGE SUCCESSFUL RECEIVED \n");
		
	 
		
	}


	/*
	//Split char array into multiple variables
	char * sta, *enn;
	
	//Allocate memory and copy first char from buffer input
	sta = malloc(sizeof(char)*2);
	strncpy(sta,saveID,1);
	sta[1]= 0;

	//Allocate memory and copy complete DSM
	enn = malloc(sizeof(char)*strlen(saveDSM));
	strncpy(enn, saveDSM,strlen(saveDSM));
        enn+=strlen(saveDSM)-1;
	*/

	printf("START :%s\n", start);
	printf("%s\n", saveID );
	printf("%s\n", saveCols);
        printf("%s\n", saveDSM);
	printf("The lenght from dsm is: %i\n",strlen(saveDSM)); 
	printf("END : %s", end );

	

	
	int i=0;
	struct dsm matrix;
	//matrix.dsm = (char*) malloc((sizeof(recvBuff) +1 )* sizeof(char));
	//strcpy(matrix.dsm, recvBuff);

	
     
	printf("\n");
        char istring[1024];

        //DecodeUTF8BytesToChar(istring, recvBuff[0], recvBuff[1], recvBuff, 1024 );
	//printf("%s \n", istring);
   
        ticks = time(NULL);
        snprintf(sendBuff, sizeof(sendBuff), "HELLO from Cluster", ctime(&ticks));     
	//write(connfd, sendBuff, strlen(sendBuff));
	write(connfd, "Hello from cluster", strlen("Hello from cluster"));	


	printf("CONNECTION RECEIVED \n");	

	close(connfd);
	sleep(1);

    }

}


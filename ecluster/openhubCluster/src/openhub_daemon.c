/*
 * @Autor: Mauricio Pradilla H.
 * @Date: 09.04.14
 * 
 * Main file to manage DSM processing in MPI cluster
 *
 *
 *
 * */

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
#include <math.h>
#include "../include/queue.h"
#include "../include/propagation_cost.h"


void *taskIreceive(void *arg);
float calculate_propagation_cost( int** dsm, int size);
int** initializeTestDsm();

void processData(char recvBuff[2000000000],int size);
int comm_rank, comm_size, thread_level_provided;

static const int BUFF_SIZE = 200000000;


/*
struct matrix{

	int id;
	int cols;
	int **dsm;
	//Matrix *dsm;

};
*/



void *erealloc(void * oldPtr, size_t amt){

    void *v = realloc(oldPtr,amt);
    if(!v){
 	fprintf(stderr,"out of memory in realloc\n");
	exit(EXIT_FAILURE);
    }
    return v;
}

void *emalloc(size_t amt){
	
    void *v = malloc(amt);
    if(!v){
	printf("malloc failed!\n");
 	fprintf(stderr,"out of memory in malloc\n");
	v = erealloc(v,amt);
	//exit(EXIT_FAILURE);
    }
    return v;
}
	
	

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
	
	//Thread para recibir y guardar los datos
	pthread_create(&Thread_receive, (void*)NULL, (void *) taskIreceive, (void *) workQueue);	

        while(1)
	{
		printf(".");
		sleep(1); 
	}

    }
	
	/* SLAVE PROCESS */
	else{

		printf("here from slave...");

		// Get the name of the processor
		MPI_Get_processor_name(procname, &lenname);
 		
		//Create thread to receive the data
       		//pthread_t Thread_receive_R1;

		//Create queue to handle incomming data
		//dataqueue* workQueue_R1;
		int **test;
   		test = initializeTestDsm();

		float ans = calculate_propagation_cost( test, 6);
		printf("propagation Cost: %.4f\n", ans); 
		//Initialize Queue
		//worQueue_R1 = dataqueue_init();
			
		//Thread para recibir y guardar los datos
//		pthread_create(&Thread_receive_R1, (void*)NULL, (void*) task_receive_MPI, (void *) workQueue_R1);		

		// PHello from master process
		printf("Hello world from slave process %s, rank %d out of %d processors\n",procname, comm_rank, comm_size);
		
		//data_dsm* pulledData;
		/*
//		dataqueue* dataqueue_p = ((dataqueue *)arg);
		pulledData = dataqueue_remove(dataqueue_p);
		
		int id;
	        if (pulledData!=NULL){
		    
		    id= pulledData->id;
    		    //strcpy(id, pulledData->id);
		    printf("ID IN SLAVE received: %d",id);
			
		}			
		*/
		
		//	printf("waiting connections in process %s ... \n", procname);
		//	sleep(1);
		//}
	}
	
	mpi_err = MPI_Finalize();
	return 0;
}

void *taskIreceive(void *arg){

    printf("Thread to receive data initializing...\n");

    int socket_desc, client_sock, c, read_size;
    struct sockaddr_in server,client;
    static char client_message[200000000];
    static char recvBuff[200000000];

    //create Socket
    socket_desc = socket(AF_INET, SOCK_STREAM,0);
    if(socket_desc ==-1)
    {
   	printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);	
    server.sin_port = htons(5001);
 
    if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server))<0)
    {
	perror("bind failed. Error");	
    }
    puts("bind done");
    
    listen(socket_desc,3);
	
    //Accept incomming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //Accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if ( client_sock < 0 )
    {
      perror("accept failed");
    }
    puts("Connection accepted");

/*
 *

    while( (read_size = recv(client_sock, client_message, BUFF_SIZE, 0))>0)
    {
	if(read_size==BUFF_SIZE)
	{
		printf("!!!!!!!!!!!limit buffer\n");
	
	}
        printf("size read:%i\n", read_size);
	sleep(20);
	
	processData(client_message,read_size);	
	
	//send mesage back to client
	write(client_sock, client_message, strlen(client_message));    
    }
    if(read_size==0)
    {
	puts("Client disconnectd");
	fflush(stdout);
    }
    else if( read_size == -1)
    { 
	perror("recv failed");
    }

   close(client_sock); 
 * */
    //char recvBuff[BUFF_SIZE];
    int actualSize=0;
    while(1) 
    {
	read_size = recv(client_sock, client_message, BUFF_SIZE, 0);
	printf("size read:%i\n", read_size);

	if(read_size>0)
	{
	    //sleep(20);
       	    printf("-%c,-%c ",client_message[0], client_message[read_size]);
/*	    
	    char * new = malloc((actualSize+read_size)*sizeof(char));
  	    memcpy(new, recvBuff, actualSize*sizeof(char));
            memcpy(new+actualSize
*/
	    //Append buffer to recvBuff
	    //recvBuff = realloc(recvBuff,(actualSize+read_size)*sizeof(char));
  	    memcpy(recvBuff+actualSize, client_message, read_size*sizeof(char)); 
/*
   	    char newArray[actualSize+read_size];
            memcpy(newArray, client_message, read_size+1);	    
	    memcpy(recvBuff, newArray, actualSize+read_size);
*/  
            actualSize+=read_size;
            printf("BUFFER: %s",recvBuff);
	
	}
	printf("last char: %c", client_message[read_size-1]);
	if(recvBuff[actualSize-1]=='$'){

       	    printf("No more data -%c,-%c ",client_message[0], client_message[read_size-1]);
            printf("%s",client_message);
	    processData(recvBuff,actualSize);	
	    //send mesage back to client
	    write(client_sock, "ok", strlen("ok"));    
    	
	    //Re initialize recv buffer and variables
	    actualSize=0;
            memset(recvBuff,0, sizeof(recvBuff));
	
	}
    	else if( read_size == -1)
    	{ 
   	    perror("recv failed");
    	}
    }
    close(client_sock); 

}

void processData(char recvBuff[200000000],int size){

    //char recvBuff[size];
    //recvBuff = irecvBuff;
    int n= 0, k;
    //char recvBuff[BUFF_SIZE];
    char *saveID, *saveCols;	
    int count = 0;
    char *start, *end, *pch;
    char copy[100];
	char *saveDSM=NULL;
	pch =NULL;
	start=NULL;
	end=NULL;
	
	pch = strtok(recvBuff, ":");
	while(pch != NULL)
	{
	    if(count==0)
	    {
	        start= malloc(sizeof(char)*(strlen(pch)+1));
		strcpy(start,pch);
	    }
	    else if(count==1)
	    {
		saveID=malloc(sizeof(char)*(strlen(pch)+1));
		strcpy(saveID,pch);
	    }
            else if(count==2)
	    {	
		printf("size of cols num:%d\n",strlen(pch));
		saveCols=(char*)malloc(sizeof(char)*(strlen(pch)+1));
		printf("malloc ok\n");
		strcpy(saveCols,pch);
		printf("strcpy ok\n");
	    }	
	    else if(count==3)
	    {
		saveDSM = emalloc(sizeof(char)*(strlen(pch)+1));
		printf("saveDSM malloc ok\n");
		strcpy(saveDSM,pch);
		printf("saveDSM strcpy ok\n");
	    }
	    else if (count==4)
	    {
		end =(char*)malloc(sizeof(char)*(strlen(pch))+1);
		strcpy(end,pch);
	    }
	    count++;
	    pch =strtok(NULL, ":");
	}

        count=0;
	//printf("%s - %s \n",start, end);
	printf("checking message integrity...\n");
	if(start!=NULL && end!=NULL && strcmp(start,"$")==0 && strcmp(end,"$")==0){	
		printf("MESSAGE SUCCESSFUL RECEIVED \n");		
	}
	else{
  		//printf("DSM: %s\n",saveDSM);	
		printf("ERROR integrity from DSM\n");	
		//exit(EXIT_FAILURE);	
//		continue;
		return NULL;
	}

	printf("START :%s\n", start);
	printf("%s\n", saveID );
	printf("%s\n", saveCols);
        //printf("%s\n", saveDSM);
	printf("The lenght from dsm is: %i\n",strlen(saveDSM)); 
		
	int i=0, j=0;
	data_dsm  *dsm;
	dsm = (data_dsm*)malloc(sizeof(data_dsm));	

	dsm->id = atoi(saveID);
	dsm->cols = atoi(saveCols);
        

	printf("Free variables...\n");
	free(saveID);
	printf("1\n");
	free(saveCols);
	printf("2\n");
	free(end);
	printf("3\n");
	free(start);
	printf("4\n");
	//free(pch);
	printf("ok\n");
        	
	int rownum;

	dsm->dsm  = malloc(sizeof(int*)*dsm->cols);
	for(rownum=0; rownum<(dsm->cols); rownum++)
	{
	    dsm->dsm[rownum] = malloc(sizeof(int)*dsm->cols);
	}
 	printf("Memory Allocated\n");	

	//dsm.dsm[0][1]=0;
	
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
	toConv = malloc(sizeof(char*));
	for(iter=0; saveDSM[iter]!='\0';iter++)
	{	
	    //printf("%c",saveDSM[iter]);
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
		if(row<(dsm->cols-1) && col<(dsm->cols-1))
		{
	           //strcpy(toConv,&saveDSM[iter]);
		   //*toConv =saveDSM[iter];
		   toInsert= atoi(&saveDSM[iter]);
		   dsm->dsm[row][col]=toInsert;
		} 
	    }
	}
	
	printf("Free dsm and conv...\n");
	free(toConv);
	toConv=NULL;
	free(saveDSM);
	saveDSM=NULL;
	//memset(saveDSM,0,sizeof(saveDSM));
	printf("ok\n");

	printf(" convert...\n");
	
	/*
        for(i=0; i<dsm->cols-1;i++)
	{
	    for(j=0;j< (dsm->cols)-1;j++)
	    {
		//matrix[i][j]= mm[i][j];
		printf("%d,", dsm->dsm[i][j] );	
	    }
	    printf(";");
	}
	*/

	//matrix.dsm = (char*) malloc((sizeof(recvBuff) +1 )* sizeof(char));
	//strcpy(matrix.dsm, recvBuff);


	//create data for queue
	//data_q = ()


	//ADD DSM TO QUEUE
	//dataqueue_add(workQueue, mm
     
	printf("\nFree memory...\n");
        printf("dsm cols: %d\n",dsm->cols);	
	    int ifree;	
	    for(ifree=0; ifree<dsm->cols-1;ifree++)
	    {
		//printf("_\n");
	        free(dsm->dsm[ifree]);	
		printf("ifree:%d\n",ifree);
	    }
	printf("Inner pointer freed\n");
	//dsm.dsm='\0';	 
	free(dsm->dsm);
	dsm->dsm = NULL;
	
	//matrix=NULL;
	//matrix=0;
	printf("ok\n");

		


}
	
int** initializeTestDsm()
{
    int **a;
    a = malloc(sizeof(int*)*6);
    int i;
    for(i=0;i<6; i++)
    {
	a[i]= (int*) malloc(sizeof(int)*6);
    }
 
    a[0][0]=0;    
    a[0][1]=1;    
    a[0][2]=1;    
    a[0][3]=0;    
    a[0][4]=0;    
    a[0][5]=0;    

    a[1][0]=0;    
    a[1][1]=0;    
    a[1][2]=0;    
    a[1][3]=1;    
    a[1][4]=0;    
    a[1][5]=0;    

    a[2][0]=0;    
    a[2][1]=0;    
    a[2][2]=0;    
    a[2][3]=0;    
    a[2][4]=1;    
    a[2][5]=0;    

    a[3][0]=0;    
    a[3][1]=0;    
    a[3][2]=0;    
    a[3][3]=0;    
    a[3][4]=0;    
    a[3][5]=0;    

    a[4][0]=0;    
    a[4][1]=0;    
    a[4][2]=0;    
    a[4][3]=0;    
    a[4][4]=0;    
    a[4][5]=1;    

    a[5][0]=0;    
    a[5][1]=0;    
    a[5][2]=0;    
    a[5][3]=0;    
    a[5][4]=0;    
    a[5][5]=0;    
    return a;
}

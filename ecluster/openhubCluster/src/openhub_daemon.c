/*
 * @Autor: Mauricio Pradilla H.
 * @Date: 09.04.14
 * 
 * Main file to manage DSM processing in MPI cluster
 *
 *
 *
 * */
#include <stddef.h>
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
#include "queue.h"
#include "propagation_cost.h"
#include "thpool.h"
#include "openhub_daemon.h"

void *taskIreceive(void *arg);
void *task_receive_MPI(void *arg);
void * task_send_MPI(void *arg);

void *analyze_data(void*arg);

float calculate_propagation_cost( int** dsm, int size);
int** initializeTestDsm();

void freeDoublePointer(int **dsm, int size);
void processData(char recvBuff[2000000000],int size, dataqueue* dataqueue_p);
int comm_rank, comm_size, thread_level_provided;

static const int BUFF_SIZE = 200000000;


/*
struct matrix{

	char id[50];
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


    /*Create a type for struct data_dsm 
    const int nitems=3;
    int blocklengths[3] = {1,1,1}; 
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, }
*/


    MPI_Comm_rank(MPI_COMM_WORLD,&comm_rank);	/* ID PROCESS IDENTIFIER */
    MPI_Comm_size(MPI_COMM_WORLD,&comm_size);	/* GET PROCESSES SIZE */
	
    char procname[MPI_MAX_PROCESSOR_NAME];
    int lenname;
    int position=0;	

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
        workQueue = dataqueue_init();
	
	//Create thread pool to send data
//	thpool_t* threadpool_Isend;
//	threadpool_Isend = thpool_init(4);	


	//Thread para recibir y guardar los datos
	pthread_create(&Thread_receive, (void*)NULL, (void *) taskIreceive, (void *) workQueue);	
	

	
	pthread_t Thread_send1;
	pthread_t Thread_send2;
	pthread_t Thread_send3;
	pthread_t Thread_send4;

	
	parameters_item *params;
 	params = (parameters_item *) malloc(sizeof(parameters_item));
	params->queue = workQueue;
	
	params->sensor = 1;
	pthread_create(&Thread_send1, (void*)NULL, (void *) task_send_MPI, (void *)params);
 	/*sleep(1);
	params->sensor = 2;
	pthread_create(&Thread_send2, (void*)NULL, (void *) task_send_MPI, (void *)params);
 	sleep(1);
	params->sensor = 3;
	pthread_create(&Thread_send3, (void*)NULL, (void *) task_send_MPI, (void *)params);
 	sleep(1);
	params->sensor = 4;
	pthread_create(&Thread_send4, (void*)NULL, (void *) task_send_MPI, (void *)params);
*/

	while(1){
		sleep(10);
	}
   	
        free(workQueue);
	/*
        while(1)
	{
 	     if(workQueue->size >0)
	     {
		thpool_add_work(threadpool_Isend, (void *) task_send_MPI, (void *)workQueue);
	     }	
		printf(".");
		sleep(1); 
	}
	pthread_join(Thread_receive, NULL);
        
	*/
	}
	/* SLAVE PROCESS */
	else{

		printf("here from slave...");
		// Get the name of the processor
		MPI_Get_processor_name(procname, &lenname);

		printf("Hello world from slave process %s, rank %d out of %d processors\n",procname, comm_rank, comm_size);
		//Create thread to receive the data
       		//pthread_t Thread_receive_R1;

		//Create queue to handle incomming data
//		dataqueue* workQueue_R1;
//	        workQueue_R1 = dataqueue_init();	

		MPI_Status status;
		//data_dsm *data_recv;
		int **dsm;
 		

		int rc;
		int outmsg;
		int test[2][2];		
		int number_amount;
		char check;
		while(1){
   		    printf("waiting job...\n");

//		    MPI_Probe(0,0, MPI_COMM_WORLD, &status);
//		    MPI_Get_count(&status, MPI_INT, &number_amount);			
		    
		
		    MPI_Recv(&outmsg, 1, MPI_INT , 0, 1 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);  
		    printf("receive dsm size %i\n", outmsg);
		    
		    dsm = malloc((sizeof(int*))*outmsg);
	            int y=0;
                    for(y=0; y<(outmsg);y++)
	            {
      		       dsm[y]=malloc((sizeof(int*))*outmsg);
	             } 

		    int dd[outmsg][outmsg];
		    MPI_Recv(&(dd[0][0]), outmsg*outmsg, MPI_INT, 0, 1, MPI_COMM_WORLD, &status); 
			

		    int a,b;
		    for(a=0;a<outmsg;a++){

			for(b=0;b<outmsg;b++){
				printf("%i",dd[a][b]);
				if(dd[a][b]!=1 && dd[a][b]!=0){
					printf("********************************  ");
					printf("index: %i, %i, value:%i\n",a,b, dd[a][b]);	
					}
				}
			    printf("\n");
			}





	//	    freeDoublePointer(dsm, outmsg);	

		//    int buffer[number_amount];
		//    MPI_Unpack( buffer, number_amount, MPI_PACKED, 0 , 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
	
		   // data_recv = (data_dsm*)malloc(sizeof(data_dsm)*number_amount);
		  //  rc = MPI_Recv(&data_recv, sizeof(data_dsm*), MPI_BYTE , 0, 1 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);  

		//    printf("I receive %i number from 0\n", number_amount);

		//    printf("JOB RECEIVED id: %i\n", data_recv->id);
		    //printf("dsm: %i", data_recv->dsm[0][0]);
		    //DO ANALYSIS over received data
		 //   int **test;
   		 //   test = initializeTestDsm();

		  //  float ans = calculate_propagation_cost(dsm,outmsg);
		 //   printf("propagation Cost: %.4f\n", ans);
		    //freeDoublePointer(test,6);		    


		    check ='a';
//		    free(data_recv);
 		    //Send Message to indicate task finished and to receive new task
		    MPI_Send(&check, 1, MPI_CHAR, 0 , 1 , MPI_COMM_WORLD);

			
		
		    printf("\nFree memory...\n");

        	//    printf("dsm cols: %d\n",outmsg);	
/*	            int ifree;	
	    	    for(ifree=0; ifree<outmsg;ifree++)
	    	    {
	        	free(dsm[ifree]);	
	            }
	            printf("Inner pointer freed\n");
		    free(dsm);
*/

		}
 
		//Initialize Queue
		//worQueue_R1 = dataqueue_init();
/*			
		//params definition
		parameters_item *params;
 		params = (parameters_item *) malloc(sizeof(parameters_item));
		params->queue = workQueue_R1;
		params->sensor = comm_rank;

		//Thread para recibir y guardar los datos
		//pthread_create(&Thread_receive_R1, (void*)NULL, (void*) task_receive_MPI, (void *)params );		

		// PHello from master process
	
		//ANALYZE IN NODE ELEMENTS IN QUEUE
		analyze_data((void*)params);
*/


	
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

    dataqueue *dataqueue_p = ((dataqueue*)arg);


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

    int actualSize=0;
    while(1) 
    {
	read_size = recv(client_sock, client_message, BUFF_SIZE, 0);
	if(read_size>0)
	{
       	    printf("-%c,-%c ",client_message[0], client_message[read_size]);
  	    memcpy(recvBuff+actualSize, client_message, read_size*sizeof(char)); 
            actualSize+=read_size;
	}
	if(recvBuff[actualSize-1]=='$'){

       	    printf("No more data -%c,-%c ",client_message[0], client_message[read_size-1]);

	    //Call method to process reveived data and add it to queue
	    processData(recvBuff,actualSize, dataqueue_p);	

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
    free(dataqueue_p);
}

void *task_send_MPI(void *arg){

    int node;
    MPI_Request request;
    MPI_Status status;

    data_dsm* pulledData;
    data_dsm* data_send;
    char inmsg='x';
    int position=0;
    int test[2][2]; 
    int **test2;
    int ss;

    parameters_item *params = ((parameters_item *)arg);
    dataqueue *dataqueue_p = params->queue;
    node = params->sensor;
    printf("THE ID FORM THREAD: %i\n", node);  
   
    while(1)
    {    
   	 pulledData = dataqueue_remove(dataqueue_p);
    
         if(pulledData != NULL){
   
   	     printf("DATA pulled from queue in node %i\n", node);
		
	
	     data_send = malloc(sizeof(*pulledData));
	     /*data_send->dsm = malloc(sizeof(int*)*pulledData->cols);
	     int y=0;
             for(y=0; y<(pulledData->cols);y++)
	     {
      		data_send->dsm[y]=malloc(sizeof(int*)*pulledData->cols);
	     } 
	     //memcpy(data_send->dsm, pulledData->dsm, sizeof(data_send->dsm));
	*/
	     memcpy(data_send, pulledData, sizeof(*data_send));
	     
	    // data_send = pulledData;

	     int colss = pulledData->cols;
             printf("- - num of cols %i\n", colss);

	     
	    // memcpy(data_send->id,pulledData->id, sizeof(data_send->id));	
	     
	     char sp[50];
	     //strcpy(sp, pulledData->id);
  	     
	
             int p;	
	     for(p=0; p<50;p++){
               printf("%c", data_send->id[p]);
	     }
	     printf("\n");

//	     memcpy(data_send->dsm, pulledData->dsm, sizeof(data_send->dsm));


	     //printf("%i", pulledData->dsm[0][0]);
	     //printf("%i", pulledData->dsm[1][0]);
//	     printf("%i", data_send->dsm[1][1]);
	     //printf("%i", pulledData->dsm[0][1]);
	     int matrix[colss][colss];
	
	     int i,j;
	     for(i=0;i<colss; i++){
		for(j=0;j<colss;j++){
		    //memcpy(matrix[i][j], pulledData->dsm[i][j], sizeof(matrix[i][j]));
		    matrix[i][j] = data_send->dsm[i][j];
		    //matrix[i][j] = 0;
	     	    printf("%i", data_send->dsm[i][j]);
		    //printf("%i", pulledData->dsm[i][j]);
		}
		printf("\n");
	     }
//             sleep(2);	
 	     //free(pulledData);            


         	test2 = initializeTestDsm();
	     //SEND job to NODE 
		test[0][0]= 1;
		test[0][1]= 0;
		test[1][0]= 0;
		test[1][1]=1;
            // test = initializeTestDsm();
	     ss = 2;	   
	     MPI_Send(&colss, 1, MPI_INT, node, 1 , MPI_COMM_WORLD);		
	     MPI_Send(&(matrix[0][0]), (colss)*(colss), MPI_INT, node, 1, MPI_COMM_WORLD);
	     printf("Job send from node 0, thread num %i\n", node);
	     printf("Waiting response from node %i...\n", node);
	     MPI_Recv(&inmsg, 1, MPI_CHAR, node, 1 , MPI_COMM_WORLD, &status);	
	     printf("Response received! from node %i with msg:%c\n", node,&inmsg);

             freeDoublePointer(test2,6); 
	     //free(test2);
	     //freeDoublePointer(data_send->dsm,data_send->cols);
	     free(data_send);
	
	     freeDoublePointer(pulledData->dsm, pulledData->cols);
	     //free(pulledData);
/*
	  //   free(pulledData);
	     MPI_Send(&inmsg, 1, MPI_CHAR, node, 1 , MPI_COMM_WORLD);		
	     printf("Job send from node 0, thread num %i\n", node);
	     printf("Waiting response from node %i...\n", node);
	     MPI_Recv(&inmsg, 1, MPI_CHAR, node, 1 , MPI_COMM_WORLD, &status);	
	     printf("Response received! from node %i with msg:%c\n", node,&inmsg);
 	*/ 
	} 	

    }   
        free(dataqueue_p);
	free(params);
      	
	//MPI_Isend(data_send, sizeof(data_dsm), MPI_BYTE, 1, 0, MPI_COMM_WORLD, &request1);

   return(NULL);
}

void freeDoublePointer(int **dsm, int size){
	int ifree;	
	for(ifree=0; ifree<size;ifree++)
	{
	    free(dsm[ifree]);	
        }
        printf("Inner pointer freed\n");
        free(dsm);

}


void *task_receive_MPI(void * arg){


  return(NULL);
}

void *analyze_data(void* arg){
    parameters_item *paramsR1 = ((parameters_item*)arg);
    int sensor = paramsR1->sensor;
    printf("Analyze Job start in sensor: %i", sensor);
    
    return(NULL);
}





void processData(char recvBuff[200000000],int size, dataqueue* dataqueue_p){

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

	char sha[50]= "not initialized...";
	
	//memcpy(sha, &saveID, strlen(sha));
	strncpy(sha,saveID,strlen(saveID)*sizeof(char));
	//sha[49]='\0';
	memcpy(dsm->id, sha, sizeof(dsm->id));	

	//strcpy(dsm->id, saveID);
        
	int p;	
	for(p=0; p<50;p++){
               printf("%c",dsm->id[p]);
	     }
	     printf("\n");
	
	//dsm->id = ;
	dsm->cols = atoi(saveCols);
        

	printf("Free variables...\n");
	free(saveID);
	//printf("1\n");
	free(saveCols);
	//printf("2\n");
	free(end);
	//printf("3\n");
	free(start);
	//printf("4\n");
	//free(pch);
	printf("ok\n");
        	
	int rownum;

	dsm->dsm  = malloc(sizeof(int*)*dsm->cols);
	for(rownum=0; rownum<(dsm->cols); rownum++)
	{
	    dsm->dsm[rownum] = malloc(sizeof(int)*(dsm->cols));
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
		   toInsert= atoi(&saveDSM[iter]);
		   dsm->dsm[row][col]=toInsert;
		} 
	    }
	}
	
	printf("Free dsm and conv...\n");
	free(saveDSM);
	//saveDSM=NULL;
	//memset(saveDSM,0,sizeof(saveDSM));
	printf("ok\n");

	printf(" convert...\n");
	


	//ADD DSM TO QUEUE
	/*printf("JOB ADDED TO QUEUE with id:");
	     int p;
	     char cp[50];
	     memcpy(cp,dsm->id, 50);
	     for(p=0; p<50;p++){
               printf("%c", cp[p]);
	     }
	     printf("\n");*/
	dataqueue_add(dataqueue_p,dsm);
    
       /* 
	printf("\nFree memory...\n");
        printf("dsm cols: %d\n",dsm->cols);	
	    int ifree;	
	    for(ifree=0; ifree<dsm->cols-1;ifree++)
	    {
		//printf("_\n");
	        free(dsm->dsm[ifree]);	
	//	printf("ifree:%d\n",ifree);
	    }
	printf("Inner pointer freed\n");
	//dsm.dsm='\0';	 
	free(dsm->dsm);
	dsm->dsm = NULL;
	*/
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

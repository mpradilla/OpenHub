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
void *taskIsend(void *arg);
void *task_receive_MPI(void *arg);
void * task_send_MPI(void *arg);

void *analyze_data(void*arg);

float calculate_propagation_cost( int** dsm, int size);
int** initializeTestDsm();

void freeDoublePointer(int **dsm, int size);
void processData(char recvBuff[2000000000],int size, dataqueue* dataqueue_p);
int comm_rank, comm_size, thread_level_provided;

static const int BUFF_SIZE = 200000000;
int ready=0;
int hugeMatrix[5000][5000];

//#define PYTHON_SERVER 10.0.1.118
#define IN_PORT  5001
#define OUT_PORT 2003

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
       
	//Create Thread to send Analysis results
	pthread_t Thread_send;

 
	//Create Queue to handle incoming data
	dataqueue* workQueue;	
        workQueue = dataqueue_init();
	
	//Create Queue send analysis results
	dataqueue* responseQueue;
	responseQueue = dataqueue_init();

	//Thread to receive and save work into queue 
	pthread_create(&Thread_receive, (void*)NULL, (void *) taskIreceive, (void *) workQueue);	
	
	//Thread to send Analysis results
	pthread_create(&Thread_send, (void*)NULL, (void *) taskIsend, (void *) responseQueue);	
	
	pthread_t Thread_send1;
	pthread_t Thread_send2;
	pthread_t Thread_send3;
	pthread_t Thread_send4;

	
	parameters_item *params;
 	params = (parameters_item *) malloc(sizeof(parameters_item));
	params->queue = workQueue;
        params->results = responseQueue;
	
	params->sensor = 1;
	pthread_create(&Thread_send1, (void*)NULL, (void *) task_send_MPI, (void *)params);
 	sleep(3);
	params->sensor = 2;
	pthread_create(&Thread_send2, (void*)NULL, (void *) task_send_MPI, (void *)params);
 	sleep(3);
	params->sensor = 3;
	pthread_create(&Thread_send3, (void*)NULL, (void *) task_send_MPI, (void *)params);
 	sleep(3);
	params->sensor = 4;
	pthread_create(&Thread_send4, (void*)NULL, (void *) task_send_MPI, (void *)params);


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
		    
		    int **dsm;
		    dsm = (int**)malloc((sizeof(int*))*outmsg);
	            int y=0;
                    for(y=0; y<(outmsg);y++)
	            {
      		       dsm[y]=(int*)malloc(sizeof(int)*outmsg);
	            }

//		    int dd[outmsg][outmsg];
		    MPI_Recv(&(hugeMatrix[0][0]), outmsg*outmsg, MPI_INT, 0, 1, MPI_COMM_WORLD, &status); 
			

		    //COPY FROM 2D Array to double pointer structre
		    //int (*dsm)[outmsg][outmsg];
		    //dsm = &dd;
 
		    int a,b;
		    for(a=0;a<outmsg;a++){

			for(b=0;b<outmsg;b++){
				
				dsm[a][b]=hugeMatrix[a][b];
				if(hugeMatrix[a][b]!=1 && hugeMatrix[a][b]!=0){
					//memcpy(dsm[a][b],dd[a][b],sizeof(int));
					printf("********************************  ");
					printf("index: %i, %i, value:%i\n",a,b, dsm[a][b]);	
					}
				//printf("%i",dsm[a][b]);
				}
			    //printf("\n");
			}

		
		    //DO ANALYSIS over received data
		    int **test;
   		    test = initializeTestDsm();

		    float ans = calculate_propagation_cost(dsm,outmsg);
		    printf("propagation Cost: %.4f\n", ans);
		    freeDoublePointer(test,6);		    
		    freeDoublePointer(dsm, outmsg);	

//		    free(data_recv);
 		    //Send Message to indicate task finished and to receive new task
		    MPI_Send(&ans, 1, MPI_FLOAT, 0 , 1 , MPI_COMM_WORLD);

			
                    //free(dsm);	  	   
	
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
    
    ready=1;

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
       	    //printf("-%c,-%c ",client_message[0], client_message[read_size]);
  	    memcpy(recvBuff+actualSize, client_message, read_size*sizeof(char)); 
            actualSize+=read_size;
	}
	if(recvBuff[actualSize-1]=='$'){

       	    //printf("No more data -%c,-%c ",client_message[0], client_message[read_size-1]);

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
    //data_dsm* data_send;
    char inmsg='x';
    int position=0;
    int test[2][2]; 
    int **test2;
    int ss;
    float response;

    parameters_item *params = ((parameters_item *)arg);
    dataqueue *dataqueue_p = params->queue;
    dataqueue *response_queue = params->results;
    node = params->sensor;
	

    printf("THE ID FORM THREAD: %i\n", node);  
   
    while(1)
    {    
   	 pulledData = dataqueue_remove(dataqueue_p);
    
         if(pulledData != NULL){
   
   	     printf("DATA pulled from queue in node %i\n", node);
		
	  //   data_send = malloc(sizeof(data_dsm*));
/*	     data_send->dsm = (int**)malloc(sizeof(int*)*pulledData->cols);
	     int y=0;
             for(y=0; y<(pulledData->cols);y++)
	     {
      		data_send->dsm[y]=(int*)malloc(sizeof(int)*pulledData->cols);
	     } 
	     memcpy(data_send->dsm, pulledData->dsm, sizeof(data_send->dsm));
*/		
	   //  memcpy(data_send, pulledData, sizeof(*data_send));
	     
	     //data_send = pulledData;
	     int colss = pulledData->cols;
             printf("- - num of cols %i\n", colss);

	     
	    // memcpy(data_send->id,pulledData->id, sizeof(data_send->id));	
	     
	     char sp[50];
	     //strcpy(sp, pulledData->id);
  	     
	
             int p;	
	     for(p=0; p<50;p++){
               printf("%c", pulledData->id[p]);
	     }
	     printf("\n");

//	     memcpy(data_send->dsm, pulledData->dsm, sizeof(data_send->dsm));


	     //printf("%i", pulledData->dsm[0][0]);
	     //printf("%i", pulledData->dsm[1][0]);
//	     printf("%i", data_send->dsm[1][1]);
	     //printf("%i", pulledData->dsm[0][1]);
//	     int matrix[colss][colss];
	     
	     printf("copy dsm to fixed array\n");

	     //memcpy(matrix,pulledData->dsm, sizeof(matrix)); 

 	
	     int ii,jj;
	     for(ii=0;ii<colss; ii++){
		for(jj=0;jj<colss;jj++){
		    //  memcpy(matrix[ii][jj], &(pulledData->dsm[ii][jj]), sizeof(matrix[ii][jj]));
		    if(sizeof(pulledData->dsm[ii][jj])==sizeof(int)){
			hugeMatrix[ii][jj] = pulledData->dsm[ii][jj];
		        //printf("%d", pulledData->dsm[ii][jj]);
		    }
		    else{ 
			printf("-_-_ Error: In POS i:%i j:%i", ii,jj);
		     }
		      //matrix[ii][jj] = 1;
	     	  //  printf("%i", matrix[i][j]);
		}
		//printf("\n");
	     }
//             sleep(2);	
 	     //free(pulledData);            

	
  	//       	test2 = initializeTestDsm();
	     //SEND job to NODE 
	//	test[0][0]= 1;
	//	test[0][1]= 0;
	//	test[1][0]= 0;
	//	test[1][1]=1;
            // test = initializeTestDsm();
	     ss = 2;	   
	     MPI_Send(&colss, 1, MPI_INT, node, 1 , MPI_COMM_WORLD);		
	     MPI_Send(&(hugeMatrix[0][0]), (colss)*(colss), MPI_INT, node, 1, MPI_COMM_WORLD);
	     printf("Job send from node 0, th:read num %i\n", node);
	     printf("Waiting response from node %i...\n", node);
	     MPI_Recv(&response, 1, MPI_FLOAT, node, 1 , MPI_COMM_WORLD, &status);	
	     printf("Response received! from node %i with response:%.3f\n", node,response);

            
	     data_dsm *resp;
	     resp = (data_dsm*)malloc(sizeof(data_dsm));
             resp->result= response;
	     resp->analysis = pulledData->analysis;
	     memcpy(resp->id, &(pulledData->id), sizeof(resp->id));		
   	     dataqueue_add(response_queue, resp);

             //freeDoublePointer(test2,6); 
	     //free(test2);
	     //freeDoublePointer(data_send->dsm,data_send->cols);
	
	     //freeDoublePointer(data_send->dsm, data_send->cols);
	     //freeDoublePointer(pulledData->dsm, pulledData->cols);
//	     free(data_send);
	     free(pulledData);
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
	free(response_queue);
      	
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

	dsm->dsm  = (int**)malloc(sizeof(int*)*dsm->cols);
	for(rownum=0; rownum<(dsm->cols); rownum++)
	{
	    dsm->dsm[rownum] = (int*)malloc(sizeof(int)*(dsm->cols));
	}
 	printf("Memory Allocated\n");	

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
		if(row<(dsm->cols) && col<(dsm->cols))
		{
		   toInsert= atoi(&saveDSM[iter]);
		   char term;
		   if(toInsert==0 || toInsert==1){
            	       dsm->dsm[row][col]=toInsert;
	           }
		   else{
		       printf("WARNING:: could not read dependency value, 0 inserted in position instead");
		       dsm->dsm[row][col]=0;
		   }
		} 
	    }
	}
	
	printf("Free dsm and conv...\n");
	free(saveDSM);
	printf("ok\n");

	printf(" convert...\n");
	
        dsm->analysis=0;
	dsm->result=-1;

	//ADD DSM TO QUEUE
	dataqueue_add(dataqueue_p,dsm);
	
	printf("ok\n");
}
	
void *taskIsend(void *arg){

    printf("Thread to send results  initializing...\n");

    int sockfd = 0, n = 0;
    struct sockaddr_in servaddr, clientaddr;
    char buffer[1024];

    dataqueue *dataqueue_p = ((dataqueue*)arg);
    data_dsm *pulledData;

    while(ready==0){
	 sleep(3);
    }
    sleep(17);    


    //create Socket
    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if( sockfd<0 ){
   	printf("\n@@@@@@@@@ Could not create the socket!! \n");
	
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr("157.253.238.116");
    servaddr.sin_port = htons(OUT_PORT);

    if(connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0){
	printf("@@@@@@@@@@@@@ Connect failed\n");
    }
    printf("waiting for responses to send...\n");
    while(1){

   	pulledData = dataqueue_remove(dataqueue_p);
	
	if(pulledData != NULL){

	    bzero(buffer,1024);		
	    int analysis;
	    analysis = pulledData->analysis;
	    printf("Analysis metrric %i\n", analysis);
	    //n = write(sockfd,buffer, strlen(buffer)); 
	    
	    printf("Response pulled from queue, Preparing to send\n");
	    //memcpy(buffer, &pulledData->result, sizeof(float));
 	    sprintf(buffer, "%s$%i$%f",pulledData->id, pulledData->analysis, pulledData->result);
	    printf("buffer ready\n"); 
	    n = write(sockfd,buffer, sizeof(buffer)); 
	     if(n<0){
	         printf("ERROR writing to socket\n");
		 dataqueue_add(dataqueue_p, pulledData);
	     }
	     bzero(buffer,1024);
	     n = read(sockfd, buffer, 1024);
	     if(n<0)
	     {
	         printf("ERROR reading from socket\n");
	         //ADD ELEMENT AGAIN TO QUEUE 
		 dataqueue_add(dataqueue_p, pulledData);
	     }
	     else
		printf("Result SEND!!!! :) :) :)\n");
	
//	     printf("%s\n",buffer);	
        }
	else
	     sleep(1);
    }
    free(pulledData);
    

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

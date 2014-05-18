#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "mpi.h"
#include <time.h>
#include <math.h>
#include "../include/clustering_cost.h"



#define ALPHA 20
#define LAMD 2 


void printDSM(int**dsm,int size);
float calculate_clustering_cost(int **dsm, int size);

void calculateVerticalBuses(int **dsm, int *size, int alpha, int **buses, int *busesSize, int **coordsX,int **coordsY, int *coordsSize, int **coordsProjected, int *coordsProjectedSize);

//Aux methods

void remove_element(int * array, int sizeOfArray, int indexToRemove);
void onerealloc(int **oldPtr, int actualSize);
int** testDSM();
int isInList(int input, int* list, int size); 

void printArray(int *arr, int size);

//Dinamic array for DSM vertical buses. Arrray with the matrix indexes
int *buses;


float calculate_clustering_cost(int **InputDsm, int size)
{
    float ans = -1.0;

    int a =0;
    printf("a:%i\n",a);
    *(&a)=1;
    printf("a:%i\n",a);


    int **dsm;
    dsm = (int**) malloc(sizeof(int*)*size);
    int row;
    for(row=0;row<size;row++){
	dsm[row] = (int*)malloc(sizeof(int)*size);
    }

    //dsm = InputDsm;
    //memcpy(dsm, InputDsm,sizeof(int)*size*size);
    //Copy input dsm to dsm
    int i,j;
    for(i=0;i<size;i++){
	for(j=0;j<size;j++){
	    dsm[i][j]=InputDsm[i][j];
	}
    }



    int *buses;
    buses = malloc(sizeof(int));
    int busesSize=0;
    int *coordsX;
    int *coordsY;
    coordsX= malloc(sizeof(int));
    coordsY= malloc(sizeof(int));
    int coordsSize=0;
    int *coordsProjected;
    coordsProjected = malloc(sizeof(int));
    int coordsProjectedSize=0;
  
 
    //GET VERTICAL BUSES 
    //calculateVerticalBuses( dsm, &size, ALPHA, &buses, &busesSize, &coordsX, &coordsY, &coordsSize, &coordsProjected, &coordsProjectedSize);	
    int colCount;
    float x,y,z,w;
    
    for(i=0; i<size; i++){
	colCount=0;
	for(j=0;j<size;j++){
		
	    //printf("%i\n",dsm[i][j]);	
	    if(dsm[i][j] != 0){
		colCount +=1;
		printf("%i.%i\n", i,j);
                coordsX = realloc(coordsX,sizeof(int)*(coordsSize+1));
                coordsY = realloc(coordsY,sizeof(int)*(coordsSize+1));
	 	coordsX[coordsSize]=i;
	 	coordsY[coordsSize]=j;
		coordsSize+=1;

  		coordsProjected = realloc(coordsProjected, sizeof(int)*(coordsProjectedSize+1));
		coordsProjected[coordsProjectedSize] = j;
		coordsProjectedSize+=1;
	    }
	}
	if(colCount!=0){
	    y = (float)colCount;
	    z = (float)size;
	    w = (float) 100/z;
	    x = (float)w*y;
	    if(x>ALPHA){
   		printf("busesSize:%i\n",busesSize);		
		buses = realloc(buses,sizeof(int)*(busesSize+1));
		buses[busesSize]=i;
		busesSize+=1;
	    }
	    y,z,w,x=0; 	
	}
    }  

    printf("cooordsSize: %i \n", coordsSize);  


    //STEP 2. REMOVE DEPENDENCIES RELATED TO BUSES	

    int deletes=0;
    //int i;
    for(i=0;i<coordsSize;i++){
        printf("x 0: %i \n", coordsX[i]);  
	if((int)isInList(coordsX[i], buses, busesSize)==1)
	{
	    printf("entro\n");
	    remove_element(coordsProjected, coordsProjectedSize, i-deletes);
	    coordsProjectedSize-=1;
	    deletes+=1;	    
	}
    }



   printf("num buses: %i\n", busesSize); 
   //printf("bus 0: %i \n", buses[0]);  
   printArray(buses,busesSize);
   printArray(coordsX,coordsSize);
   printArray(coordsY,coordsSize);
   printArray(coordsProjected,coordsProjectedSize);

    printf("Clustering cost calculated. free memory...");
    free(buses);
    free(coordsX);
    free(coordsY);
    free(coordsProjected);

    for(row=0;row<size;row++)
    {
	free(dsm[row]);
    }
    free(dsm);
    printf("done\n");
	

   return ans;	
} 



// Calculate the vertical buses for given dsm doble pointer
// @Param alpha: Perecentage of use by other classes to consider it a bus
// @Param dsm: DSM to calculate the buses
// @Param size: DSM size
void calculateVerticalBuses(int **dsm, int *size, int alpha, int **buses, int *busesSize, int **coordsX,int **coordsY, int *coordsSize, int **coordsProjected, int *coordsProjectedSize){

 
    printf("dsm[1][0]:%i\n",dsm[1][0]);
	
    //int *buses;
    //int busesSize=0;

    int* buses2 = malloc(sizeof(int)*(6));
    //int *coords;
    //int *cX = malloc(sizeof(int)*(*coordsSize));
    //int *ccordsX = malloc(sizeof(int)*(*coordsSize));
    //int *cY = malloc(sizeof(int)*(*coordsSize));
    //int *ccordsY = malloc(sizeof(int)*(*coordsSize));

    //int *coordsProjected;
    //int * csP = malloc(sizeof(int)*(*coordsProjectedSize));

   //buses = buses2;  
    //free(buses);    
    int pos=0;
    int colCount;
    int i,j;
    float x,y,z,w;
    
    for(i=0; i<*size; i++){
	colCount=0;
	for(j=0;j<*size;j++){
		
	    //printf("%i\n",dsm[i][j]);	
	    if(dsm[i][j] != 0){
		colCount +=1;
		printf("%i.%i\n", i,j);
                *coordsX = realloc(*coordsX,sizeof(int)*(*coordsSize+1));
                *coordsY = realloc(*coordsY,sizeof(int)*(*coordsSize+1));
 		//*coordsX[*coordsSize]= i;  
 		//*coordsY[*coordsSize]= j;  
	 	*coordsSize+=1;
		

  		*coordsProjected = realloc(*coordsProjected, sizeof(int)*(*coordsProjectedSize+1));
		//coordsProjected[*coordsProjectedSize] = j;
		*coordsProjectedSize+=1;
		//cX = realloc(cX, sizeof(int)*(*coordsSize+1));
		//cY = realloc(cY, sizeof(int)*(*coordsSize+1));
   		//onerealloc(coordsX,(int)*coordsSize);
   		//onerealloc(coordsY,(int)*coordsSize);
		//printf("coords:%i\n",*coordsSize);		
 		//cX[*coordsSize]= i;  
		//cY[*coordsSize]= j;  
   		//onerealloc(coordsProjected,(int)*coordsProjectedSize);
		//coordsProjected[(int)*coordsProjectedSize] = j;
	    }
	}
	if(colCount!=0){
	    y = (float)colCount;
	    z = (float)(int)*size;
	    w = (float) 100/z;
	    x = (float)w*y;
	    //printf("i:\n",i);
	    //printf("x:%f in  i:%i\n",x,i);
	    if(x>alpha){
   		printf("busesSize:%i\n",*busesSize);		
		//*buses = realloc(*buses,sizeof(int)*(*busesSize));
		//onerealloc(buses, *busesSize);		
		////int *tmp = malloc((*busesSize+1)*sizeof(int));
		//*buses=*tmp;
		pos = (int) *busesSize;
		buses2[pos]=i;
                //free(tmp);
		*busesSize+=1;
		//onerealloc(buses,(int)*busesSize);
	    }
	    y,z,w,x=0; 	
	}
    }  

    //Copy response to param to send it back
    *buses = realloc(*buses,*busesSize*sizeof(int));
    *buses=buses2;
    //*coordsX = *cX;
    //*coordsY= *cY;
    //*coordsProjected=*csP;

        
    //printf("x 0: %i \n", cX[0]);  
    //printf("x 1: %i \n", cX[1]);  
    free(buses2);
/*
    free(cX);
    free(cY);
    free(csP);	
*/	
}

void  remove_element(int * array, int sizeOfArray, int indexToRemove)
{
    //int * temp = malloc((sizeOfArray -1 ) * sizeof(int));
    int c;
  
    for(c=indexToRemove;c<sizeOfArray-1;c++){
	array[c]=array[c+1];
    }
	
    /*
    //Remove last item from array
    if (indexToRemove == sizeOfArray-1 )
	memcpy(temp,array, (sizeOfArray - 1) * sizeof(int));
    else if (indexToRemove == 0 )
	memcpy(temp,array+1, (sizeOfArray - 1));
    else{
	memcpy(temp, array, (indexToRemove)* sizeof(int));
	memcpy(temp+indexToRemove, array+(indexToRemove+1), (sizeOfArray - indexToRemove));
    }
    free(array); 
    return temp;*/
} 


//Increase 1 the size of dinamic array
void onerealloc(int **oldPtr, int actualSize){

  int *temp;
  temp = realloc(*oldPtr, (actualSize+1)*sizeof(int));
   if(!temp){
	printf("Error in realloc to increase vector size in 1");
   	exit(EXIT_FAILURE);
   }
   *oldPtr=temp;
   //return oldPtr;
}  

void printArray(int *arr, int size)
{
    int i;
    for(i=0;i<size;i++)
    {
	    printf("%i,", arr[i]);
    }
    printf("\n");
}

void printDSM22(int **dsm, int size)
{
    int i,j;
    for(i=0;i<size;i++)
    {
        for(j=0;j<size;j++)
	{
	    printf("%i,", dsm[i][j]);
	}
	printf("\n");
    }
}


int isInList(int input, int *list, int size)
{
    int i;
    for(i=0;i<size; i++)
    {
	if(list[i]==input)
    	    return 1;
    }
    return 0;	
}

main()
{
    printf("Clustering Cost\n");
    int **dsm;
    dsm = testDSM();
	
    printf("dsm[1][0]:%i\n",dsm[1][0]);
    float a; 
    a = calculate_clustering_cost(dsm, 6);

    printf("C Cost: %f\n", a);

    int row;
    for(row=0;row<6;row++)
    {
	free(dsm[row]);
    }
    free(dsm);
    printf("done\n");
}

int** testDSM()
{
   int **test;
   test = malloc(sizeof(int*)*6);
   int i;
   for(i=0;i<6;i++)
   {
 	test[i]=(int*)malloc(sizeof(int)*6);
   } 

   test[0][0]=0;
   test[0][1]=0;
   test[0][2]=0;
   test[0][3]=0;
   test[0][4]=0;
   test[0][5]=0;

   test[1][0]=1;
   test[1][1]=0;
   test[1][2]=0;
   test[1][3]=0;
   test[1][4]=1;
   test[1][5]=1;

   test[2][0]=1;
   test[2][1]=0;
   test[2][2]=0;
   test[2][3]=0;
   test[2][4]=0;
   test[2][5]=0;

   test[3][0]=0;
   test[3][1]=1;
   test[3][2]=0;
   test[3][3]=0;
   test[3][4]=0;
   test[3][5]=0;

   test[4][0]=0;
   test[4][1]=0;
   test[4][2]=1;
   test[4][3]=0;
   test[4][4]=0;
   test[4][5]=0;

   test[5][0]=0;
   test[5][1]=0;
   test[5][2]=0;
   test[5][3]=0;
   test[5][4]=1;
   test[5][5]=0;

   return test;
}










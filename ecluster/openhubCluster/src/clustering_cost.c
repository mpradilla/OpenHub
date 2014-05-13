#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "mpi.h"
#include <time.h>
#include <math.h>
#include "../include/clustering_cost.h"



#define ALPHA 5
#define LAMD 2 


void printDSM(int**dsm,int size);
float calculate_clustering_cost(int **dsm, int size);

int* calculateVerticalBuses(int **dsm, int size, int alpha, int *buses, int busesSize, int *coordsX,int *coordsY, int coordsSize, int *coordsProjected, int coordsProjectedSize);

//Aux methods

int * remove_element(int * array, int sizeOfArray, int indexToRemove);
void *onerealloc(void *oldPtr, int actualSize);
int** testDSM();
int isInList(int input, int* list, int size); 


//Dinamic array for DSM vertical buses. Arrray with the matrix indexes
int *buses;


float calculate_clustering_cost(int **InputDsm, int size)
{
    float ans = -1.0;

    int **dsm;
    dsm = (int**) malloc(sizeof(int*)*size);
    int row;
    for(row=0;row<size;row++){
	dsm[row] = (int*)malloc(sizeof(int)*size);
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
    calculateVerticalBuses( dsm, size, ALPHA, buses, busesSize, coordsX, coordsY, coordsSize, coordsProjected, coordsProjectedSize);	

    int deletes=0;
    int i;
    for(i=0;i<coordsSize;i++){

	if((int)isInList(coordsX[i], buses, busesSize)==1)
	{
	    remove_element(coordsProjected, coordsProjectedSize, i-deletes);
	    coordsProjectedSize-=1;
	    deletes+=1;	    
	}

    }

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
int* calculateVerticalBuses(int **dsm, int size, int alpha, int *buses, int busesSize, int *coordsX,int *coordsY, int coordsSize, int *coordsProjected, int coordsProjectedSize){

    //int *buses;
    //int busesSize=0;

    //int *coords;
    //int coordsSize=0;
    
    //int *coordsProjected;
    //int coordsProjectedSize=0;
    
    int colCount;
    int i,j;
    float x,y,z,w;
    
    for(i=0; i<size; i++){
	colCount=0;
	for(j=0;j<size;j++){
	
	    if(dsm[j][i] != 0){
		colCount +=1;
   		onerealloc(coordsX,coordsSize);
   		onerealloc(coordsY,coordsSize);
		coordsX[coordsSize]= i;  
		coordsY[coordsSize]= j;  
	 	coordsSize+=1;
   		onerealloc(coordsProjected,coordsProjectedSize);
		coordsProjected[coordsProjectedSize] = j;
  		coordsProjectedSize+=1;
	    }
	}
	if(colCount!=0){
	    y = (float)colCount;
	    z = (float)size;
	    w = (float) 100/z;
	    x = (float)w*y;
	    if(x>alpha){
   		onerealloc(buses,busesSize);
		busesSize+=1;
	    } 	
	}
    }  
    return buses;
}

int * remove_element(int * array, int sizeOfArray, int indexToRemove)
{
    int * temp = malloc((sizeOfArray -1 ) * sizeof(int));
    
    if (indexToRemove !=0 )
	memcpy(temp,array, (indexToRemove - 1) * sizeof(int));
    if (indexToRemove != (sizeOfArray -1))
	memcpy(temp+indexToRemove, array+indexToRemove+1, (sizeOfArray - indexToRemove -1)* sizeof(int));
    free(array);
    return temp;
} 


//Increase 1 the size of dinamic array
void *onerealloc(void *oldPtr, int actualSize){

   void *v = realloc(oldPtr, sizeof(int)*actualSize+1);
   if(!v){
	printf("Error in realloc to increase vector size in 1");
   	exit(EXIT_FAILURE);
   }
   return v;
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










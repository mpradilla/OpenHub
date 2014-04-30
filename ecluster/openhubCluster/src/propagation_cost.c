#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
#include <time.h>
#include <math.h>
#include "../include/propagation_cost.h"


void printDSM(int**dsm,int size);
float calculate_propagation_cost(int **dsm, int size);


float calculate_propagation_cost(int **InputDsm, int size)
{
    float ans = -1.0;


    //Malloc memory for dsm and Copy the pointer to dsm
    int **dsm;
    dsm = (int**)malloc(sizeof(int*)*size);
    int row;
    for(row=0;row<size; row++)
    {
	dsm[row] = (int*)malloc(sizeof(int)*size);
    }
   
   //int dsm[size][size];

   int l,q;
   for(l=0;l<size;l++){
	for(q=0;q<size;q++){
	     //memcpy(dsm[l][q], InputDsm[l][q], sizeof(int));
	    dsm[l][q]=InputDsm[l][q];
	}
   }

   //dsm = InputDsm;

    //printDSM(dsm,6);

    //Calculate Succesive powers if the matrix 
    int i,j,k;
    for(i=size-1; i>=0; i--)
    {
	for(j=size-1; j>=0 ; j--)
	{
	    //Identity matrix, is always 1
	    if(j==i)
		dsm[i][j]=1;
	    else if (dsm[i][j]!=0)
	    {
		//for security avoid deps with values greater than 1
		dsm[i][j]=1;
		for(k=size-1;k>=0;k--)
		{
		    //Add trasnsitive dep for element in row i
   		    if(dsm[j][k]!=0)
			dsm[i][k]=1;
		} 
	    }
	}
    }

  //  printDSM(dsm,6);
    //calculate the Fan-Out visibility for the whole system
    float totalSum = 0;
    int Outcount=0;
    for(i=0; i<size; i++)
    {
	Outcount=0;
	for(j=0;j<size;j++)
	{
	    if(dsm[i][j]==1)
	        Outcount++;		
//	    printf("i:%i j:%i val:%i\n", i,j, dsm[i][j]);
	}
	totalSum+=Outcount;
	printf("countcol: %i\n",Outcount);
    }
 
    totalSum=totalSum*100;
    float down = size;
    down = down*down;
    ans = totalSum/down;
 
    //printf("%.2f\n", totalSum);
    //printf("%.2f\n", ans);
	
    //Free memory
    int ifree;
    for(ifree=0; ifree<size; ifree++)
    {
	free(dsm[ifree]);	
    }
    free(dsm);
    

   return ans;	
} 

void printDSM(int **dsm, int size)
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




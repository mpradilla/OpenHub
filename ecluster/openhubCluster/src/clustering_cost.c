#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "mpi.h"
#include <time.h>
#include <math.h>
#include "../include/clustering_cost.h"
#include <assert.h>


#define ALPHA 20
#define LAMD 2 


void printDSM(int**dsm,int size);
float calculate_clustering_cost(int **dsm, int size);

void calculateVerticalBuses(int **dsm, int *size, int alpha, int **buses, int *busesSize, int **coordsX,int **coordsY, int *coordsSize, int **coordsProjected, int *coordsProjectedSize);

//Aux methods

float** seq_kmeans(float**, int, int, int, float, int*);

int getIndexCluster(float *array, int size , float val);
void remove_element(int * array, int sizeOfArray, int indexToRemove);
void add_element(float * array, int sizeOfArray,float num, int indexToAdd);
void onerealloc(int **oldPtr, int actualSize);
int** testDSM();
int** testPerfectDSM();
int** testGoodDSM();
int** testRegularDSM();
int** testBadDSM(int **test);
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
    int *coordsProjectedX;
    coordsProjectedX = malloc(sizeof(int));
    int coordsProjectedSize=0;
  
 
    //STEP 1. GET VERTICAL BUSES 
    //calculateVerticalBuses( dsm, &size, ALPHA, &buses, &busesSize, &coordsX, &coordsY, &coordsSize, &coordsProjected, &coordsProjectedSize);	
    int colCount;
    float x,y,z,w;
    
    for(i=0; i<size; i++){
	colCount=0;
	for(j=0;j<size;j++){
		
	    //printf("%i\n",dsm[i][j]);	
	    if(dsm[i][j] != 0){
		colCount +=1;
		//printf("%i.%i\n", i,j);
                coordsX = realloc(coordsX,sizeof(int)*(coordsSize+1));
                coordsY = realloc(coordsY,sizeof(int)*(coordsSize+1));
	 	coordsX[coordsSize]=i;
	 	coordsY[coordsSize]=j;
		coordsSize+=1;

  		coordsProjected = realloc(coordsProjected, sizeof(int)*(coordsProjectedSize+1));
		coordsProjected[coordsProjectedSize] = i;
  		coordsProjectedX = realloc(coordsProjectedX, sizeof(int)*(coordsProjectedSize+1));
		coordsProjectedX[coordsProjectedSize] = j;
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
        //printf("x 0: %i \n", coordsX[i]);  
	if((int)isInList(coordsX[i], buses, busesSize)==1)
	{
	    //printf("entro\n");
	    remove_element(coordsProjected, coordsProjectedSize, i-deletes);
	    coordsProjectedSize-=1;
	    deletes+=1;	    
	}
    }

   printf("num buses: %i\n", busesSize); 
   printf("num coords: %i\n", coordsSize); 
   //printf("bus 0: %i \n", buses[0]);  
   printArray(buses,busesSize);
   printArray(coordsX,coordsSize);
   printArray(coordsY,coordsSize);
   printArray(coordsProjected,coordsProjectedSize);
   printArray(coordsProjectedX,coordsProjectedSize);

   //STEP 3. FIND CLUSTERS FROM DEPENDENCIES
	
   int numClusters = 3;

   if(numClusters>coordsProjectedSize){
	return -1;
   }
   float **obj;
   obj = malloc(coordsProjectedSize*sizeof(float*));
   int rr;
   for(rr=0;rr<coordsProjectedSize;rr++){
       obj[rr]= malloc(sizeof(float));
       obj[rr][0]=coordsProjected[rr]; 
   }

   int *membership;
   membership = (int*)malloc(coordsProjectedSize*sizeof(int));
   float **clusters;
   clusters =  seq_kmeans(obj, 1 ,coordsProjectedSize, numClusters, 0.001 , membership);
/*
   //Get intersection between clusters
   int **intersections;
   intersections = (int**)malloc(numClusters*sizeof(int*));
   for(row=0;row<numClusters;row++)
   {
	intersections[row]=(int*)malloc(2*sizeof(int));
   }

   //for first cluster, always go form 0
   intersections[0][0]=0;
   for(i=1;i<numClusters;i++)
   {
	mid = ((clusters[i][0]-clusters[i-1][0])/2.0)+ clusters[i-1][0];
	intersections[i-1][1]=mid;
	intersections[i][0]=mid;
   }
   intersection[numClusters-1][1]=size;
  */ 

   //SORT ASCENDING THE CENTROIDS
   //STEP 4. SORT ASCENDING THE CENTROIDS
   float *sortedCent;
   int sortSize=0;
   sortedCent = malloc(numClusters*sizeof(float));
   for(i=0;i<numClusters;i++)
	sortedCent[i]=-1;
   
   int ult =-1;
   for(i=0;i<numClusters;i++){
	if(ult!=-1){
	    for(j=0;j<sortSize;j++){
		if(clusters[i][0]<sortedCent[j]){
		    add_element(sortedCent, sortSize, (float) clusters[i][0], j);
		    //sortedCent[j]=clusters[i][0];
		    sortSize+=1;
		    ult= i;
                    break;
		}
	    }
	    if(ult!=i)
	    {
		add_element(sortedCent, sortSize, clusters[i][0], sortSize);
		//sortedCent[sortSize]=clusters[i][0];
		sortSize+=1;
	    }
	}
 	else{
 	    add_element(sortedCent, sortSize, clusters[i][0], 0);
	    //sortedCent[sortSize]=clusters[i][0];
	    sortSize+=1;
	    ult=i;
	}
   } 

  
   printf("centroid 0 %f\n", clusters[0][0]);
   printf("centroid 1 %f\n", clusters[1][0]);
   printf("centroid 2 %f\n", clusters[2][0]);
   printf("sort centroid 0 %f\n", sortedCent[0]);
   printf("sort centroid 1 %f\n", sortedCent[1]);
   printf("sort centroid 2 %f\n", sortedCent[2]);
 
   //Asign X,Y dependency coordinates to each identify cluster
   
   int **cluster;
   int *clusterSize;
   cluster = (int**)malloc((numClusters+1)*sizeof(int*));
   clusterSize = (int*)malloc((numClusters+1)*sizeof(int));
   for(row=0;row<numClusters+1;row++)
   {
       cluster[row]=(int*)malloc(sizeof(int));
       clusterSize[row]=0;
   }
   float limitA, limitB; 
   int sse=0;
   for(i=0;i<coordsProjectedSize;i++)
   {
	//CHECK first if dependency is not outside the cluster extricly in the diagonal of the dsm
	int clNum = membership[i];
	float cc = clusters[clNum][0];
        int indx;
	indx = getIndexCluster(sortedCent, numClusters , cc);
	float lastCentroid=0,centroid=0,nextCentroid=0;

	centroid=cc;
	if(indx!=0)
	    lastCentroid = sortedCent[indx-1];
	else
            lastCentroid=0.0;
 	if(indx!=numClusters-1)
	    nextCentroid = sortedCent[indx+1];
	else
            nextCentroid=size;
	    
	printf("lastC:%f  cent:%f next:%f\n",lastCentroid,centroid,nextCentroid);	

	if(lastCentroid>centroid)
	   printf("********LIST NOT ORDENED!!!");

	if(indx==0)
	   limitA=0;
	else
	   limitA = ((centroid-lastCentroid)/2.0)+lastCentroid ;

	if(indx==numClusters-1)
	   limitB=size;
	else
	   limitB = ((nextCentroid-centroid)/2.0)+centroid;

	int y = coordsProjected[i];
	int x = coordsProjectedX[i];
	printf("y:%i x:%i  limitA:%f limitB:%f\n",y, x,limitA, limitB);
 	
	//printf("limA:%d limB:%d coordY:%i coordX:%i\n", limitA,limitB, coordsProjected[i],coordsProjectedX[i]);	
	if(y>=limitA && y<=limitB && x>=limitA && x<=limitB)
        {
	    cluster[membership[i]]=realloc(cluster[membership[i]], sizeof(int)*(clusterSize[membership[i]]+1));
	    cluster[membership[i]][clusterSize[membership[i]]]=coordsProjected[i];
 	    clusterSize[membership[i]]+=1;
	}
        else
	{
	    //Dependency has no cluster, added to las cluster list for extra penatilization
	    printf("no in cluster really\n");
	    cluster[numClusters]=realloc(cluster[numClusters], sizeof(int)*(clusterSize[numClusters]+1));
	    cluster[numClusters][clusterSize[numClusters]]=coordsProjected[i];
 	    clusterSize[numClusters]+=1;
	}
 	//Evaluate number of clusters by SSE (sum of squared error)
	int  cl = membership[i];
	int yCl = clusters[cl][0];
	int dis = abs(coordsProjected[i]-yCl);	
        sse+=pow(dis,2);

	printf("cluster num:%i with size:%i - dep in y=%i centroid in:%i dis:%i sse:%i\n", membership[i],clusterSize[membership[i]], coordsProjected[i],yCl,dis,sse );
   }		
   


   /*

   //STEP 4. SORT ASCENDING THE CENTROIDS
   int *sortedCent;
   int sortSize=0;
   sortedCent = malloc(numClusters*sizeof(int));   
   int ult =-1;
   for(i=0;i<numClusters;i++){
	if(ult!=-1){
	    for(j=0;j<sortSize;j++){
		if(clusters[i][0]<sortedCent[j]){
		    sortedCent[j]=clusters[i][0];
		    sortSize+=1;
		    ult= i;
                    break;
		}
	    }
	    if(ult!=i)
	    {
		sortedCent[sortSize]=clusters[i][0];
		sortSize+=1;
	    }
	}
 	else{
	    sortedCent[sortSize]=clusters[i][0];
	    sortSize+=1;
	}
   } 
  
   */
 
   /*
   for(i=0;i<coordsSize;i++)
   {
       for(j=0;j<coordsProjectedSize;j++)
       {
           if(coordsY[i]==coordsProjected[j])
	   {
	        cluster[membership[j]]=realloc(cluster[membership[j]], sizeof(int)*(clusterSize[membership[j]]+1));
	 	cluster[membership[j]][clusterSize[membership[j]]]=i;
 		clusterSize[membership[j]]+=1;

		printf("cluster num:%i with size:%i - dep in y=%i\n", membership[j],clusterSize[membership[j]], i );
   		
		//Evaluate number of clusters by SSE (sum of squared error)
		//int  cl = membership[j];
		//int yCl = clusters[cl][0];
		//int dis = abs(coordsY[j]-yCl);	
			
           }
 	   else{
		printf("no entor\n");
	}
       }       
   }


   int sse=0;
   //OJO. en coorsSize estan todas las coordenadas, pero en membership solo las que nos son buses!! 
   //cluster al que pertenece
   for(i=0;i<numClusters;i++)
   {
       for(j=0;j<clusterSize[i];j++)
       {
           int co =cluster[i][j];
           int cl = cluster[i][0];
	   int dis = abs(co-cl);
           sse+=pow(dis,2);
	   printf("coord num:%i with y=%i from cluster %i with centroid %i and dis %i. Total %i\n", j, co, i , cl, dis, sse);
	   	 
       }
   }
*/

   
 /* 
   printf("cluster %i with x:%i\n",0, cluster[0][0]);
   printf("cluster %i with x:%i\n",1, cluster[1][1]);
   printf("cluster %i with x:%i\n",2, cluster[2][0]);
*/	   


  //CLUSTERING COST
  //IMPORTANT! DEPENDENCEIS OUTSIDE ANY CLUSTER ARE IN cluster[NumCluster], that is in last "cluster"
  float cost =0;
  float clusterCost;
  float depCost;
  for(i=0;i<numClusters+1;i++)
  {  clusterCost=0; 

      for(j=0;j<clusterSize[i];j++)
      {
          depCost=0;
	  if(isInList(cluster[i][j],buses,busesSize)==1)
	     depCost =1;
 	  else
	  {
	      //cluster[i][j] show the j depdendency, based on the coordsX,Y numeration from cluster number i
	      if((isInList(cluster[i][j],cluster[i],clusterSize[i]==1)) && i!=(numClusters))
	          depCost = pow(clusterSize[i],LAMD);
  	      else
	          depCost = pow(size, LAMD);
	  }
      clusterCost+=depCost;
      }
      cost+=clusterCost;
  }


  printf("CLUTERING COST: %f\n", cost);
  ans = cost;
 
    for(row=0;row<coordsProjectedSize;row++)
    {
	free(obj[row]);
	//printf("Memeber %i\n", membership[row]);
    }
    free(obj);

    for(row=0;row<numClusters+1;row++)
    {
	//printf("clusters %i with x:%f\n",row, clusters[row][0]);
//	printf("orde clusters %i with x:%f\n",row, sortedCent[row]);
	free(cluster[row]);
    }
    free(membership);
    free(sortedCent);

    //free(cluster[0]);
    free(cluster);
    free(clusterSize);
    free(clusters[0]);
    free(clusters);


    printf("Clustering cost calculated. free memory...");
    free(buses);
    free(coordsX);
    free(coordsY);
    free(coordsProjected);
    free(coordsProjectedX);

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
int getIndexCluster(float *array, int size , float val)
{
    int ans = -1;
    int i;
    for(i=0;i<size;i++)
    {
	if(array[i]==val)
	   return i;
    }

    return -1;
}


void add_element(float * array, int sizeOfArray, float num, int indexToAdd)
{
    //*array = realloc(*array,(sizeOfArray +1 ) * sizeof(int));  
    int c;

        if(indexToAdd==-1)
	    indexToAdd=0;
    
        for(c=indexToAdd+1;c<sizeOfArray+1;c++){
	    if(c==0)
	      continue;
	    else   
	       array[c]=array[c-1];
    	}
        array[indexToAdd]=num;
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
    float a; 
/*
    dsm = testRegularDSM();
    a = calculate_clustering_cost(dsm, 6);
    printf("C regular Cost: %f\n", a);
    
   dsm = testGoodDSM();
    a = calculate_clustering_cost(dsm, 6);
    printf("C Good Cost: %f\n", a);
    */
    dsm = testPerfectDSM();
    a = calculate_clustering_cost(dsm, 20);
    printf("C Perfect Cost: %f\n", a);
    
    dsm = testBadDSM(dsm);
    a = calculate_clustering_cost(dsm, 20);
    printf("C Bad Cost: %f\n", a);
    
    int row;
    for(row=0;row<20;row++)
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
int** testPerfectDSM()
{
   
   int **test;
   test = malloc(sizeof(int*)*20);
   int i,j;
   for(i=0;i<20;i++)
   {
 	test[i]=(int*)malloc(sizeof(int)*20);
   } 

int testa[20][20]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		   0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   		   0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   		   0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  		   0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   		   0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   		   0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   		   0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
   		   0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,
   		   0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,
   		   0,0,0,0,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,
   		   0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
  		   0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,
   		   0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,
   		   0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,0,0,0,0,
   		   0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,0,0,0,0,
   		   0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,0,0,
   		   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,
   		   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,
   		   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1};
   for(i=0;i<20;i++){
	for(j=0;j<20;j++)
	{
 	    test[i][j]=testa[i][j];
	}
   } 

  return test;
}
//USE FROM
/*
int** testGoodDSM(int ** test)
{
   
   int **test;
   test = malloc(sizeof(int*)*20);
   int i;
   for(i=0;20;i++)
   {
 	test[i]=(int*)malloc(sizeof(int)*20);
   } 
   

   test[0] =  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[1] =  {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[2] =  {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[3] =  {0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[4] =  {0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[5] =  {0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[6] =  {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[7] =  {0,0,0,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0};
   test[8] =  {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0};
   test[9] =  {0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0};
   test[10] = {0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0};
   test[11] = {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0};
   test[12] = {0,0,0,1,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0};
   test[13] = {0,0,0,1,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,0};
   test[14] = {0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,1,0,0,0};
   test[15] = {0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0};
   test[16] = {0,0,0,1,0,0,1,0,0,0,1,0,1,1,1,0,1,0,0,0};
   test[17] = {0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,1,0,1,0,0};
   test[18] = {0,0,0,1,0,0,1,0,0,1,0,0,0,0,1,1,0,0,1,0};
   test[19] = {0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1};
   return test;

}
int** testRegularDSM()
{
   int **test;
   test = malloc(sizeof(int*)*20);
   int i;
   for(i=0;20;i++)
   {
 	test[i]=(int*)malloc(sizeof(int)*20);
   } 

   test[0] =  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[1] =  {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[2] =  {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[3] =  {0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[4] =  {0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[5] =  {0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   test[6] =  {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0};
   test[7] =  {0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0};
   test[8] =  {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0};
   test[9] =  {0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0};
   test[10] = {0,0,0,0,0,0,1,1,1,0,1,0,1,1,0,0,0,0,0,0};
   test[11] = {0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0};
   test[12] = {1,0,0,0,0,0,0,0,0,0,1,0,1,1,0,0,0,1,0,0};
   test[13] = {0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0};
   test[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0};
   test[15] = {0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,1,0};
   test[16] = {0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,0,0};
   test[17] = {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0};
   test[18] = {0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0};
   test[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};
   return test;

}
*/
int** testBadDSM(int **test)
{
   int i,j;
   /*int **test;
   test = malloc(sizeof(int*)*20);
   for(i=0;20;i++)
   {
 	test[i]=(int*)malloc(sizeof(int)*20);
   } */
   int tes[20][20]={{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   	     {0,1,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
   	     {0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   	     {0,0,1,1,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0},
   	     {0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0},
   	     {0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
   	     {0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
             {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0},
   	     {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
   	     {0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,0,0,0},
   	     {0,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0},
   	     {0,0,0,0,0,0,0,0,0,1,0,1,0,0,1,1,0,1,1,0},
   	     {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,0},
             {0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0},
   	     {0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0},
   	     {0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,0,0,0},
   	     {0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
   	     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0},
   	     {0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,0},
  	     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}};
   for(i=0;i<20;i++){
	for(j=0;j<20;j++)
	{
 	    test[i][j]=tes[i][j];
	}
   } 
   return test;

}
//USE FROM

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*  Wei-keng Liao                                                            */
/*            ECE Department, Northwestern University                        */
/*            email: wkliao@ece.northwestern.edu                             */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*----< euclid_dist_2() >----------------------------------------------------*/
/* square of Euclid distance between two multi-dimensional points            */
__inline static
float euclid_dist_2(int    numdims,  /* no. dimensions */
                    float *coord1,   /* [numdims] */
                    float *coord2)   /* [numdims] */
{
    int i;
    float ans=0.0;
    
    for (i=0; i<numdims; i++)
        ans += (coord1[i]-coord2[i]) * (coord1[i]-coord2[i]);
    
    return(ans);
}

/*----< find_nearest_cluster() >---------------------------------------------*/
__inline static
int find_nearest_cluster(int     numClusters, /* no. clusters */
                         int     numCoords,   /* no. coordinates */
                         float  *object,      /* [numCoords] */
                         float **clusters)    /* [numClusters][numCoords] */
{
    int   index, i;
    float dist, min_dist;
    
    /* find the cluster id that has min distance to object */
    index    = 0;
    min_dist = euclid_dist_2(numCoords, object, clusters[0]);
    
    for (i=1; i<numClusters; i++) {
        dist = euclid_dist_2(numCoords, object, clusters[i]);
        /* no need square root */
        if (dist < min_dist) { /* find the min and its array index */
            min_dist = dist;
            index    = i;
        }
    }
    return(index);
}

/*----< seq_kmeans() >-------------------------------------------------------*/
/* return an array of cluster centers of size [numClusters][numCoords]       */
float** seq_kmeans(float **objects,      /* in: [numObjs][numCoords] */
                   int     numCoords,    /* no. features */
                   int     numObjs,      /* no. objects */
                   int     numClusters,  /* no. clusters */
                   float   threshold,    /* % objects change membership */
                   int    *membership)   /* out: [numObjs] */
{
    int      i, j, index, loop=0;
    int     *newClusterSize; /* [numClusters]: no. objects assigned in each
                              new cluster */
    float    delta;          /* % of objects change their clusters */
    float  **clusters;       /* out: [numClusters][numCoords] */
    float  **newClusters;    /* [numClusters][numCoords] */
    
    /* allocate a 2D space for returning variable clusters[] (coordinates
     of cluster centers) */
    clusters    = (float**) malloc(numClusters *             sizeof(float*));
    assert(clusters != NULL);
    clusters[0] = (float*)  malloc(numClusters * numCoords * sizeof(float));
    assert(clusters[0] != NULL);
    for (i=1; i<numClusters; i++)
        clusters[i] = clusters[i-1] + numCoords;
    
    /* pick first numClusters elements of objects[] as initial cluster centers*/
    for (i=0; i<numClusters; i++)
        for (j=0; j<numCoords; j++)
            clusters[i][j] = objects[i][j];
    
    /* initialize membership[] */
    for (i=0; i<numObjs; i++) membership[i] = -1;
    
    /* need to initialize newClusterSize and newClusters[0] to all 0 */
    newClusterSize = (int*) calloc(numClusters, sizeof(int));
    assert(newClusterSize != NULL);
    
    newClusters    = (float**) malloc(numClusters *            sizeof(float*));
    assert(newClusters != NULL);
    newClusters[0] = (float*)  calloc(numClusters * numCoords, sizeof(float));
    assert(newClusters[0] != NULL);
    for (i=1; i<numClusters; i++)
        newClusters[i] = newClusters[i-1] + numCoords;
    
    do {
        delta = 0.0;
        for (i=0; i<numObjs; i++) {
            /* find the array index of nestest cluster center */
            index = find_nearest_cluster(numClusters, numCoords, objects[i],
                                         clusters);
            
            /* if membership changes, increase delta by 1 */
            if (membership[i] != index) delta += 1.0;
            
            /* assign the membership to object i */
            membership[i] = index;
            
            /* update new cluster centers : sum of objects located within */
            newClusterSize[index]++;
            for (j=0; j<numCoords; j++)
                newClusters[index][j] += objects[i][j];
        }
        
        /* average the sum and replace old cluster centers with newClusters */
        for (i=0; i<numClusters; i++) {
            for (j=0; j<numCoords; j++) {
                if (newClusterSize[i] > 0)
                    clusters[i][j] = newClusters[i][j] / newClusterSize[i];
                newClusters[i][j] = 0.0;   /* set back to 0 */
            }
            newClusterSize[i] = 0;   /* set back to 0 */
        }
        
        delta /= numObjs;
    } while (delta > threshold && loop++ < 500);
    
    free(newClusters[0]);
    free(newClusters);
    free(newClusterSize);
    
    return clusters;
}








//
// gcc -fopenmp ompDemo.c
//
#include <stdio.h>
// #include <omp.h>
#include <math.h>
//
#define N 4
#define COLS 8
//


void sumArray( int A[][COLS] )
{
   printf("Sum:\n");
   int rank , size , x;
   int t = N;
   for(x = 0; x < 4; x++)
   {   
      omp_set_num_threads( t ) ;
      #pragma omp parallel private(rank) ///4, 2, 1
      { 
         rank = omp_get_thread_num() ;
         if(x < 3)
            A[N-x-2][rank] = A[N-x-1][2*rank] + A[N-x-1][2*rank+1];
      }
      t = t/2;
      int i;
      for(i = 0; i < 8; i++)
         printf("%d ", A[N-x-1][i] );
      printf("\n");
   }

}

void prefixSum(int A[][COLS], int C[][COLS])
{
   printf("Prefix:\n");
   int rank , size , x ;
   int t = 1;
   for(x = 0; x < 4; x++) //0, 1, 2
   {   
      omp_set_num_threads( t ) ;
      #pragma omp parallel private(rank) ///4, 2, 1
      { 
         rank = omp_get_thread_num() ; /// this is equal to i
         if(rank == 0)
            C[x][rank] = A[x][rank];
         else if(rank%2 == 0 && rank > 0) //odd according to paper
            C[x][rank] = A[x][rank] + C[x-1][rank/2-1];
         else if(rank%2 != 0 && rank > 0) //even according to paper
            C[x][rank] = C[x-1][rank/2];
      }
      t = t*2;
      int i;
      for(i = 0; i < 8; i++)
         printf("%d ", C[x][i] );
      printf("\n");
   }
} 

void compaction(int bits[][COLS], int A[][COLS], int C[][COLS], int out[])
{
   printf("Compaction:\n");
   int sum = bits[0][0];
   int rank, i;
   omp_set_num_threads( COLS ) ;
   #pragma omp parallel private(rank)
      { 
         i = omp_get_thread_num();
         if(bits[3][i] == 1)
            out[C[3][i]-1] = A[3][i];
      }   

   for(i = 0; i < 8; i++)
         printf("%d ", out[i] );
}

void compaction2(int bits[][COLS], int A[][COLS], int C[][COLS], int out[])
{
   printf("CompactionIndex:\n");
   int sum = bits[0][0];
   int rank, i;
   omp_set_num_threads( COLS ) ;
   #pragma omp parallel private(rank)
      { 
         i = omp_get_thread_num();
         if(bits[3][i] == 1)
            out[C[3][i]-1] = i;
      }   

   for(i = 0; i < 8; i++)
         printf("%d ", out[i] );
}

void mergeSortHelper(int A[], int B[], int out[], int low, int high, int threads)
{ 
   int rank, r, i;
   omp_set_num_threads( threads ) ;
   #pragma omp parallel private(rank)
      { 
         i = omp_get_thread_num();
         ///binarySearch for Rank
         if(B[i] < A[0])
            r = low;
         else if(B[i] > A[high-1])
            r = high;
         else
         {
            int min = low;
            int max = high;
            int key = B[i];
            int mid;
            while((max - min) != 1)
            {
               mid = (max+min)/2;
               if(A[mid] < key)
                  min = mid;
               else
                  max = mid;
            }
            r = min+1;
            // printf("%d %d \n" , r, key);
         }
         out[r+i] = B[i];
      } 
}
void mergeSort(int A[], int B[], int out[], int low, int high)
{
   mergeSortHelper(A, B, out, low, high, high);
   mergeSortHelper(B, A, out, low, high, high);
   int i;
   printf("Merge Sort:\n");
   for(i = 0; i < 2*high; i++)
         printf("%d ", out[i] );
   printf("\n");

}

void nearestOne(int prefix[][COLS], int comp[], int nearOne[])
{
   printf("Nearest One:\n");
   int rank, i;
   omp_set_num_threads( 8 ) ;
   #pragma omp parallel private(rank)
      { 
         i = omp_get_thread_num();
         nearOne[i] = comp[prefix[3][i]-1];
      }

   for(i = 0; i < 8; i++)
         printf("%d ", nearOne[i] );
}

void partition(int A[], int B[], int out[], int low, int high)
{
   int partitions = 16/(log(16)/log(2));
   printf("%d\n", partitions );
   int i, rank, r;
   int pivots[4][2] = {}; /// variable can't be placed in index?
   omp_set_num_threads( partitions ) ;
   #pragma omp parallel private(rank)
      { 
         i = 4*omp_get_thread_num();
         ///binarySearch for Rank
         if(B[i] < A[0])
            r = low;
         else if(B[i] > A[high-1])
            r = high;
         else
         {
            int min = low;
            int max = high;
            int key = B[i];
            int mid;
            while((max - min) != 1)
            {
               mid = (max+min)/2;
               if(A[mid] < key)
                  min = mid;
               else
                  max = mid;
            }
            r = min+1;
         }
         out[r+i] = B[i];
         pivots[i/4][0] = r;
         pivots[i/4][1] = i;
      } 

   for(i = 0; i < partitions; i++)
      printf("%d %d ", pivots[i][0], pivots[i][1] );
   printf("\n");
   omp_set_num_threads(partitions) ;
   #pragma omp parallel private(rank)
      {
         i = omp_get_thread_num();
         int r = pivots[i][0];
         i = pivots[i][1];
         int pointerA = r;
         int pointerB = i+1;
         int pointerC = r+i+1;
         while(out[pointerC] == 0) 
         {
            int min;
            if(A[pointerA] < B[pointerB] || pointerB > 15)
               {
               min = A[pointerA];
               pointerA++;
               }
            else if(A[pointerA] > B[pointerB] || pointerA > 15)
               {
               min = B[pointerB];
               pointerB++;
               }
            out[pointerC] = min;
            pointerC += 1;
         }
      }
   int pointerC = 0;
   int pointerA = 0;
   while(out[pointerC] == 0) 
      {
         out[pointerC] = A[pointerA];
         pointerA++;
         pointerC++;
      }

   for(i = 0; i < 2*high; i++)
      printf("%d ", out[i] );
   printf("\n");
}


int main( int argc , char* argv[] )
{
   // int data[4][8] = {{0,0,0,0,0,0,0,0},
   //                   {0,0,0,0,0,0,0,0},
   //                   {0,0,0,0,0,0,0,0},
   //                   {1,1,2,3,5,2,1,2}}; // sum = 1,2,4,7,12,14,15,17


   int prefix[4][8] = {{0,0,0,0,0,0,0,0},
                       {0,0,0,0,0,0,0,0},
                       {0,0,0,0,0,0,0,0},
                       {0,0,0,0,0,0,0,0}}; 
  
   int bits[4][8] = {{0,0,0,0,0,0,0,0},
                     {0,0,0,0,0,0,0,0},
                     {0,0,0,0,0,0,0,0},
                     {1,0,1,1,0,1,0,0}};  //prefix sum: 1,1,2,3,3,4,4,4
                                          //compaction: 0,2,3,5

   //int out[8] = {0,0,0,0,0,0,0,0};
 ////////// Sum and Prefix Sum
   // sumArray(A);

   // printf("\n\n");

   // prefixSum(A, C);
 ////////// Compaction
   // sumArray(bits);

   // printf("\n\n");

   // prefixSum(bits, C);

   // printf("\n\n");
   
   // compaction(bits, A, C, out);

   // printf("\n\n");

////////// Merge Sort


   // int A[8] = {1,2,3,6,8,12,13,16};
   // int B[8] = {4,5,7,9,10,11,14,15};
   // int out[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

   // mergeSort(A, B, out, 0, 8);
   
   
/////////  Nearest One

   // int comp[8] = {0,0,0,0,0,0,0,0};
   // int nearOne[8] = {0,0,0,0,0,0,0,0};
   // sumArray(bits);
   // printf("\n\n");
   // prefixSum(bits, prefix);
   // printf("\n\n");
   // compaction2(bits, prefix, prefix, comp);
   // printf("\n\n");
   // nearestOne(prefix, comp, nearOne);

   // printf("\n\n");

///////  Partition
   int A[16] = {1,2,3,6,8,12,13,16,19,20,23,25,26,28,31,32};
   int B[16] = {4,5,7,9,10,11,14,15,17,18,21,22,24,27,29,30};
   int out[32] = {};
   partition(A, B, out, 0, 16);
   //mergeSort(A, B, out, 0, 16);



   

}

// Goal : Write a recursive threading met hod to find the defective bulbs and the number of threads that have been created for this purpose

#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_ARRAY_SIZE 100

int array[MAX_ARRAY_SIZE];
int threadcount = 1;

// A structure to keep the needed information of each thread
typedef struct
{
int* array;
int arraySize;

} ThreadInfo;

bool ContainsDefective(int* array, int size);
void* FindDefective(void *a);

int main()
{
  ThreadInfo info;
  pthread_t threadid;
  int array_size, index=0;

  printf("%d\n", threadcount);
  // Read input from file
  FILE *ptr_file;
  int i;

  ptr_file = fopen("input.txt","r");
  if (!ptr_file) {
     printf("Could not open file\n");
     return 1;
   }

  // Get array size
  fscanf (ptr_file, "%d", &array_size);
  printf("array size = %d\nContent = ", array_size);

  // Assign input to array
  while (!feof (ptr_file))
  {
   fscanf (ptr_file, "%d", &i);
   array[index] = i;
   if(index < array_size)
    printf ("%d ", i);
   index++;
  }
  printf("\n");
  // initializing thread info
  info.array = array;
  info.arraySize = array_size;

  pthread_create(&threadid, NULL,  FindDefective,  (void *)&info);

  // Wait for thread to finish
  pthread_join(threadid, NULL);
  printf("Total threads created = %d\n", threadcount);

  fclose(ptr_file);
  return 0;
}

bool ContainsDefective(int* array, int size)
{
  int i;
  for(i = 0;i < size; ++i)
  {
    if(array[i] == false)
      return true;
  }

  return false;
}

// The start address (function) of the threads
void* FindDefective(void* a)
{
  pthread_t leftchild_id, rightchild_id;
  ThreadInfo* info = (ThreadInfo*)a;
  // Increment thread count
  threadcount++;
  printf("Thread number %d\n", threadcount);

  // Keep information about current thread
  printf("Current array size = %d\n", info->arraySize);

  for(int i =0; i < info->arraySize; ++i)
  {
    printf("%d ", *(info->array + i));
  }
  printf("\n");

  // if there are no children, check if the lightbulb is defective
  if(info->arraySize == 1)
  {
    int value = *(info->array);
    printf("Light bulb value = %d\n", value);
    if( value == 0 ) {
      /*
      Keep index
      */
      pthread_exit(0);
    }
    else {
      // Not defective, move on
      pthread_exit(0);
    }
  }

  // If no light bulb are defective, end thread
  if(!ContainsDefective(info->array, info->arraySize)) {
     printf("No defective light bulb found\n");
     pthread_exit(0);
  }

  // Divide array
  int leftArraySize = info->arraySize / 2;
  int rightArraySize = info->arraySize / 2;
  printf("left array size %d\n", leftArraySize);
  printf("right array size %d\n", rightArraySize);

  // Instantiate 2 child objects
  ThreadInfo* leftChild = new ThreadInfo;
  ThreadInfo* rightChild = new ThreadInfo;

  if(leftChild == NULL || rightChild == NULL) {
    printf("created null child\n"); 
    pthread_exit(0);
  }

  leftChild->array = info->array;
  leftChild->arraySize = leftArraySize;

  rightChild->array = (info->array + leftArraySize);
  rightChild->arraySize = rightArraySize;

  // Spawn 2 threads
  pthread_create(&leftchild_id, NULL,  FindDefective,  (void *)&leftChild);
  pthread_create(&rightchild_id, NULL,  FindDefective,  (void *)&rightChild);

  // wait for child processes to end
  pthread_join(leftchild_id, NULL);
  pthread_join(rightchild_id, NULL);
  
  pthread_exit(0); /* exit */
}

// Goal : Write a recursive threading met hod to find the defective bulbs and the number of threads that have been created for this purpose

// To compile: Must be under GNU/Linux
// In terminal: g++ assignment1.cpp -lpthread -o assignment1

// To run: ./assignment1

#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <vector>

#define MAX_ARRAY_SIZE 100

int array[MAX_ARRAY_SIZE];
int defectiveArray[MAX_ARRAY_SIZE];
int threadcount = 0;
int defectivecount = 0;

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
  printf("array size = %d\n", array_size);

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
  printf("Total threads created = %d\nDefective lightbulb at index: ", threadcount);

  for (int y = 0; y < defectivecount; y++) {
    printf("%d ", defectiveArray[y]);
  }
  printf("\n");

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
  int leftArraySize, rightArraySize;

  // Keep information about current thread
  ThreadInfo* info = (ThreadInfo*)a;
  // Increment thread count
  threadcount++;

  // if there are no children, check if the lightbulb is defective
  if(info->arraySize == 1)
  {
    int value = *(info->array);
    // printf("Light bulb value = %d\n", value);
    if( value == 0 ) {
      // Keep index
      int index = info->array - array;

      defectiveArray[defectivecount] = index;
      defectivecount++;
      pthread_exit(0);
    }
    else {
      // Not defective, move on
      pthread_exit(0);
    }
  }

  // If no light bulb are defective, end thread
  if(!ContainsDefective(info->array, info->arraySize)) {
     pthread_exit(0);
  }

  // Instantiate 2 child objects
  ThreadInfo* leftChild = new ThreadInfo;
  ThreadInfo* rightChild = new ThreadInfo;

  if(leftChild == NULL || rightChild == NULL) {
    printf("created null child\n");
    pthread_exit(0);
  }

  // Divide array
  if(info->arraySize % 2 == 0)
  {
    leftArraySize = info->arraySize / 2;
    rightArraySize = info->arraySize / 2;
  }
  else
  {
    leftArraySize = info->arraySize / 2;
    rightArraySize = leftArraySize + 1;
  }

  leftChild->array = info->array;
  leftChild->arraySize = leftArraySize;

  rightChild->array = (info->array + leftArraySize);
  rightChild->arraySize = rightArraySize;

  // Create 2 threads
  // wait for child processes to end
  pthread_create(&leftchild_id, NULL,  FindDefective, leftChild);
  pthread_join(leftchild_id, NULL);
  pthread_create(&rightchild_id, NULL,  FindDefective, rightChild);
  pthread_join(rightchild_id, NULL);


  pthread_exit(0); /* exit */
}

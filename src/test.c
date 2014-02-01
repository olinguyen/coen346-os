#include<stdio.h>

int main()
{
  FILE *ptr_file;
  int i;

  ptr_file =fopen("input.txt","r");
  if (!ptr_file)
      return 1;

  while (!feof (ptr_file))
  {
    fscanf (ptr_file, "%d", &i);
    printf ("%d\n", i);
  }


  fclose(ptr_file);
      return 0;
}

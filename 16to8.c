
/* convert RAW 16bits samples into 8 bits */

#include<stdio.h>

int main(int argc, char *argv[])
{
  unsigned char c;
  
  fgetc(stdin);
  c = fgetc(stdin);
  while(!feof(stdin)) {
    putchar(c);

    fgetc(stdin);
    c = fgetc(stdin);
	}    
  

}

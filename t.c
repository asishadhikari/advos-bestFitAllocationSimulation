#include <stdio.h>
#include <stdlib.h>

int main(void) {
  for (int i = 0; i< 200; i++){
    printf("Hi!");
//    i = 198;
	i += 198;
  }

  return 0;
}

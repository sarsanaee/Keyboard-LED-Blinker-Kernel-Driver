#include <stdio.h>

int main(int argc, char *argv[] )  {

   if( argc >= 2 ) 
   {
      printf("The argument supplied is %s\n", argv[1]);
      if(strcmp(argv[1],"r") == 0)
      {
          FILE *fp;
          char buff[1024];
          fp = fopen("/dev/mysimpledriver", "r");
          fscanf(fp, "%s", buff);
          fclose(fp);  
      }
      else if(strcmp(argv[1],"w") == 0)
      {
          FILE *fp;
          fp = fopen("/dev/mysimpledriver", "w+");
          //fprintf(fp, argv[2]);
          fputs(argv[2], fp);
          fclose(fp);
      }
   }
   else {
      printf("Unexpected Arguments.\n");
   }
   
   return 0;

}

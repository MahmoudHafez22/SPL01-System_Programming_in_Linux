#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main(int argc ,char* argv[])
{
        if(argc != 3)
        {
                printf("Usage: mv %s file name \n",argv[0]);
                exit(-1);
        }
        if((rename(argv[1],argv[2])) != 0 )
        {
                printf("Error\n");
                exit(-2);
        }
        return 0 ;
}

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define COUNT  100
int main(int argc ,char* argv[])
{
        int buf[COUNT];
        if(argc != 2)
        {
                printf("Usage: %s file name \n",argv[0]);
                exit(-1);
        }
        int fd= open(argv[1],O_RDONLY);
        if(fd <0 )
        {
                printf("could not open a file\n");
                exit(-2);
        }
        int num;
        while((num = read(fd, buf, COUNT)) > 0)
        {
          if(write(1, buf, num)<0)
			{
                printf("Write failed\n");
                exit(-3);
            }
        }
        close(fd);
        return 0 ;
}

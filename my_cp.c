#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define COUNT  1000
int main(int argc ,char* argv[])
{
        int buf[COUNT];
        if(argc != 3)
        {
                printf("Usage: %s file name \n",argv[0]);
                exit(-1);
        }
        int open_fd= open(argv[1],O_RDONLY);
        if(open_fd <0 )
        {
                printf("could not open a file\n");
                exit(-2);
        }
        int dest_fd = open(argv[2],O_WRONLY | O_CREAT | O_TRUNC,0644);
	if(dest_fd < 0)
	{
		printf("could not open a destination file\n");
                close(open_fd);
		exit(-3);
	}
	int n_read,n_write;
        while((n_read = read(open_fd, buf, COUNT)) > 0)
        {
		n_write = write(dest_fd,buf,n_read);
          	if(n_write != n_read)
		{
                	printf("Write failed\n");
			close(open_fd);
			close(dest_fd);
               		exit(-3);
            	}
        }
        close(open_fd);
	close(dest_fd);
        return 0 ;
}
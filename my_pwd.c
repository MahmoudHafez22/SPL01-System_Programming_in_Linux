#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

#define COUNT 1000
int main()
{
char *buf;
buf = (char *)malloc(COUNT);
if((getcwd(buf , COUNT)) != NULL)
{
	printf("%s\n",buf);
}
else{
printf("error");
}
free(buf);
return 0;
}
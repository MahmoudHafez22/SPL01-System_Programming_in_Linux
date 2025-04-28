#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define BUF_SIZE 1000
int main(int argc, char *argv[]) {
    int status = 0;
    char buf[BUF_SIZE];
    //printf("FemtoShell\n");
    while(1){
        printf("Femtoshell > ");
        if(fgets(buf,BUF_SIZE,stdin) != 0){
            if(strcmp(buf,"exit\n")==0){
                printf("Good Bye\n");
                exit(status);
                break;
            }
            else if(strncmp(buf,"echo ",5)==0){
                printf("%s",buf+5);
            }
            else if(strcmp(buf,"\n")==0){

            }
            else{

                printf("Invalid command\n");
                status = -1 ;
            }
        }
        else{
            exit(status);
            break;
        }
    }
    return 0;

}




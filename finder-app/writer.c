#include <stdio.h>
#include <syslog.h>
#include <errno.h>

int main(int argc, char **argv){
    openlog(NULL,0,LOG_USER);
    if (argc!=3){
        syslog(LOG_ERR,"Invalid number of arguments %d",argc);
        return 1;
    }

    FILE *file=fopen(argv[1],"wb");
    if(file==NULL){
        syslog(LOG_ERR,"Failed to open file %s",argv[1]);
        perror("Failed to open file");
        return 1;
    }

    syslog(LOG_DEBUG,"Writing %s to %s",argv[2],argv[1]);
    fprintf(file,argv[2]);
    fclose(file);
    return 0;
}

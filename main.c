#include <stdio.h>
#include <string.h>

#include "dircheck.h"


/* We are taking first argument as initial path name. */
int main(int argc, char *argv[])
{
    if ((strcmp(argv[1],"-s") == 0) || (strcmp(argv[1],"--save") == 0)){
        if(argc != 4){
            fprintf(stderr,"Wrong parameters\n");
            return 1;
        }
        if(generate_xml(argv[2],argv[3]) != 0){
            fprintf(stderr,"Error couldn't create xml file\n");
            return 1;
        }
        return 0;
    }


    if ((strcmp(argv[1],"-c") == 0) || (strcmp(argv[1],"--check") == 0)){
        if(argc != 3) {
            fprintf(stderr,"Wrong parameters\n");
            return 1;
        }
        if(read_xml(argv[2]) != 0) {
            fprintf(stderr,"Error couldn't read xml file\n");
            return 1;
        }
        return 0;
    }

    fprintf(stderr,"Wrong parameters\n");
    return 1;
}


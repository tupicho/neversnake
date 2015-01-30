#include <stdio.h>


void tablarank() {
    FILE *fp;

    char buffer[100];

    fp = fopen ( "asd.txt", "r" );

    fscanf(fp, "%s" ,buffer);
    printf("%s",buffer);

    fclose ( fp );

}
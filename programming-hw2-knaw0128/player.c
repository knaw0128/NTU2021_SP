#include<stdlib.h>
#include<stdio.h>
#include<string.h>
int main(int argc, char **argv){
    if (argc < 3) {
       fprintf(stderr,"usage stuff...%s %s", argv[0], argv[0]);
       exit(0);
    }
    int guess;
    char tmp[100];
    for(int a=1;a<11;a++){
        srand ((atoi(argv[2]) + a) * 323);
        guess = rand() % 1001;
        printf("%s %d\n", argv[2], guess);
    }
    return 0;
}
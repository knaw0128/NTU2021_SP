#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

int scoreboard[20];
int host_num;
int n;
int pids[10];

int min(int a, int b){
    return (a<b)?a:b;
}

int handle_read(int fd){
    char host_num[100];
    int idx=0;
    while(idx==0||host_num[idx]!='\n'){
        read(fd, &host_num[idx++], 1);
    }
    host_num[idx]='\0';
    fprintf(stderr, " new end host num %s\n", host_num);
    int ans = atoi(host_num);

    for(int a=0;a<8;a++){
        char player[100], score[100];
        int idxp=0, idxc=0;
        while (idxp==0||player[idxp]!=' '){
            read(fd, &player[idxp++], 1);
        }
        player[idxp]='\0';
        while (idxc==0||score[idxc]!='\n'){
            read(fd, &score[idxc++], 1);
        }
        score[idxc] = '\0';
        fprintf(stderr, "new ended   player %d  score %d \n",atoi(player), atoi(score));
        scoreboard[atoi(player)] += atoi(score);
    }
    return ans;
}

void show(){
    for(int a=1;a<=n;a++){
        printf("player  %d  score : %d\n", a, scoreboard[a]);
    }
}

int main(int argc, char **argv){ //host num , player num, lucky num
    if(argc<4){
        fprintf(stderr, "wrong argument num\n");
        return 0;
    }
    host_num = atoi(argv[1]);
    n = atoi(argv[2]);
    int fifo = open("fifo_0.tmp", O_RDWR);
    int mission[1000][8], idx=1;

    for(int a=1;a<=n-7;a++){
        for(int b=a+1;b<=n-6;b++){
            for(int c=b+1;c<=n-5;c++){
                for(int d=c+1;d<=n-4;d++){
                    for(int e=d+1;e<=n-3;e++){
                        for(int f=e+1;f<=n-2;f++){
                            for(int g=f+1;g<=n-1;g++){
                                for(int h=g+1;h<=n;h++){
                                    mission[idx][0] = a;
                                    mission[idx][1] = b;
                                    mission[idx][2] = c;
                                    mission[idx][3] = d;
                                    mission[idx][4] = e;
                                    mission[idx][5] = f;
                                    mission[idx][6] = g;
                                    mission[idx++][7] = h;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    idx--;

    fd_set ready;
    for(int a=1;a<=min(idx, host_num);a++){
        char fifo_name[100];
        sprintf(fifo_name, "fifo_%d.tmp", a);
        int input = open(fifo_name, O_RDWR);
        for(int b=0;b<7;b++){
            char player[10];
            sprintf(player, "%d ", mission[a][b]);
            write(input, player, strlen(player));         
        }
        char player[10];
        sprintf(player, "%d\n", mission[a][7]);
        write(input, player, strlen(player));

        int pid = fork();
        if(pid==0){
            char host_id[10];
            sprintf(host_id, "%d", a);
            dup2(input, 0);
            dup2(fifo, 1);
            execlp("./host", "./host", "-m", host_id, "-d", "0", "-l", argv[3], (char *)0);
        }
        // close(input);
    }

    fprintf(stderr, "----------- first part done -----------------\n");

    for(int a=min(idx, host_num)+1;a<=idx;a++){
        wait(NULL);
        int done_host = handle_read(fifo);
        fprintf(stderr, " now at %d, next host = %d\n", a, done_host);

        char fifo_name[100];
        sprintf(fifo_name, "fifo_%d.tmp", done_host);
        int input = open(fifo_name, O_RDWR);
        for(int b=0;b<7;b++){
            char player[10];
            sprintf(player, "%d ", mission[a][b]);
            write(input, player, strlen(player));           
        }
        char player[10];
        sprintf(player, "%d \n", mission[a][7]);
        write(input, player, strlen(player));

        int pid = fork();
        if(pid==0){
            char host_id[10];
            sprintf(host_id, "%d", done_host);
            dup2(input, 0);
            dup2(fifo, 1);
            fprintf(stderr, "host num going to be exec = %d\n", done_host);
            execlp("./host", "./host", "-m", host_id, "-d", "0", "-l", argv[3], (char *)0);
        }
        close(input);
    }

    show();

    return 0;
}
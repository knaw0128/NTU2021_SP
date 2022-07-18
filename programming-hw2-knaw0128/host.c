#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

int abs(int k){
    return (k>0)?k:-k;
}

int find_same(char *tar){
    if(strlen(tar)!=2||tar[0]!='-')
        return -1;
    if(tar[1]=='m')
        return 0;
    if(tar[1]=='d')
        return 1;
    if(tar[1]=='l')
        return 2;
    return -1;
}

void record_score(int player, int *names, int *scoreboard){
    for(int a=0;a<8;a++)
        if(player==names[a]){
            scoreboard[a]+=10;
            return;
        }
}

int main(int argc, char **argv){
    
    if (argc != 7) {
       fprintf(stderr,"usage stuff...%s %s", argv[0], argv[0]);
       exit(0);
    }

    char host_id[20], depth[20], lucky_num[20];
    int pid;
    int forked[2]={0,0};
    for(int a=1;a<7;a+=2){
        if(find_same(argv[a])==0){
            for(int b=0;b<strlen(argv[a+1]);b++)
                host_id[b] = argv[a+1][b];
            host_id[strlen(argv[a+1])] = '\0';
        }
        else if(find_same(argv[a])==1){
            for(int b=0;b<strlen(argv[a+1]);b++)
                depth[b] = argv[a+1][b];
            depth[strlen(argv[a+1])] = '\0';
        }
        else if(find_same(argv[a])==2){
            for(int b=0;b<strlen(argv[a+1]);b++)
                lucky_num[b] = argv[a+1][b];
            lucky_num[strlen(argv[a+1])] = '\0';
        }
        else{
            printf("wrong parameter\n");
            return 0;
        }
    }
    
    int fdto[2][2], fdget[2][2];
    if(pipe(fdto[0])<0){
        fprintf(stderr, "pipe to  err\n");
        return 0;
    }
    if(pipe(fdget[0])<0){
        fprintf(stderr, "pipe get err\n");
        return 0;
    }
    if(pipe(fdto[1])<0){
        fprintf(stderr, "pipe to  err\n");
        return 0;
    }
    if(pipe(fdget[1])<0){
        fprintf(stderr, "pipe get err\n");
        return 0;
    }


    while(depth[0]=='0'){
        char next_depth[2];
        next_depth[0] = depth[0]+1;
        next_depth[1] = '\0';
        
        char playerID[8][20];
        int playerID_num[8];
        int scoreboard[8]={0};
        for(int a=0;a<8;a++){
            scanf("%s", playerID[a]);
            playerID_num[a] = atoi(playerID[a]);
            scoreboard[a] = 0;
        }

        if(playerID_num[0]==-1)
            return 0;

        char winner[2][10][20], score[2][10][20];


        for(int b=0;b<2;b++){
            
            for(int c=0;c<4;c++){
                write(fdto[b][1], playerID[b*4+c], strlen(playerID[b*4+c]));
                write(fdto[b][1], " ", 1);
            }
            write(fdto[b][1], "\n", 1);

            if(!forked[b]){
                forked[b]=1;
                pid = fork();
                if(pid<0){
                    fprintf(stderr, "fork err\n");
                    return 0;
                }
                if(pid==0){
                    close(fdto[b][1]);
                    close(fdget[b][0]);
                    dup2(fdto[b][0], 0);
                    dup2(fdget[b][1], 1);
                    execlp("./host", "./host", "-m", host_id, "-d", next_depth, "-l", lucky_num, (char *)0);
                }
            }
                        
            for(int a=0;a<10;a++){
                int nidx1=0;
                while(nidx1==0||winner[b][a][nidx1-1]!=' '){
                    int get = read(fdget[b][0], &winner[b][a][nidx1++], 1);
                    if(get==0)
                        nidx1--;
                }
                winner[b][a][nidx1-1] = '\0';
                nidx1=0;
                while(nidx1==0||score[b][a][nidx1-1]!='\n'){
                    int get = read(fdget[b][0], &score[b][a][nidx1++], 1);
                    if(get==0)
                        nidx1--;
                }
                score[b][a][nidx1-1] = '\0';
            }
            
        }

        for(int a=0;a<10;a++){
            int lu = atoi(lucky_num);
            int s0 = atoi(score[0][a]);
            int s1 = atoi(score[1][a]);
            if(abs(s0-lu)<abs(s1-lu)||(abs(s0-lu)==abs(s1-lu)&&atoi(winner[0][a])<atoi(winner[1][a])))
                record_score(atoi(winner[0][a]), playerID_num, scoreboard);
            else
                record_score(atoi(winner[1][a]), playerID_num, scoreboard);
        }

        printf("%s\n", host_id);
        for(int a=0;a<8;a++){
            printf("%s %d\n", playerID[a], scoreboard[a]);
        }
    }

    while(depth[0]=='1'){
        char next_depth[2];
        next_depth[0] = depth[0]+1;
        next_depth[1] = '\0';

        char studentID[4][20];
        char players[2][10][100];
        char scores[2][10][100];
        for(int a=0;a<4;a++)
            scanf("%s", studentID[a]);

        for(int a=0;a<2;a++){
            write(fdto[a][1], studentID[a*2], strlen(studentID[a*2]));
            write(fdto[a][1], " ", 1);
            write(fdto[a][1], studentID[a*2+1], strlen(studentID[a*2+1]));
            write(fdto[a][1], "\n", 1);

            
            if(!forked[a]){
                forked[a] = 1;
                pid = fork();
                if(pid<0){
                    fprintf(stderr, "fork err\n");
                    return 0;
                }
                if(pid==0){
                    close(fdto[a][1]);
                    close(fdget[a][0]);
                    dup2(fdto[a][0], 0);
                    dup2(fdget[a][1], 1);
                    execlp("./host", "./host", "-m", host_id, "-d", next_depth, "-l", lucky_num, (char *)0);
                } 
            }
            for(int b=0;b<10;b++){
                int nidx1=0;
                while(nidx1==0||players[a][b][nidx1-1]!=' '){
                    int get = read(fdget[a][0], &players[a][b][nidx1++], 1);
                    if(get<=0)
                        nidx1--;
                }
                players[a][b][nidx1-1] = '\0';
                nidx1=0;
                while(nidx1==0||scores[a][b][nidx1-1]!='\n'){
                    int get = read(fdget[a][0], &scores[a][b][nidx1++], 1);
                    if(get<=0)
                        nidx1--;
                }
                scores[a][b][nidx1-1] = '\0';
            }
            
        }


        for(int a=0;a<10;a++){
            int lu = atoi(lucky_num);
            int s0 = atoi(scores[0][a]);
            int s1 = atoi(scores[1][a]);
            if(abs(s0-lu)<abs(s1-lu)||(abs(s0-lu)==abs(s1-lu)&&atoi(players[0][a])<atoi(players[1][a])))
                printf("%s %s\n", players[0][a], scores[0][a]);
            else
                printf("%s %s\n", players[1][a], scores[1][a]);
        }        
        fflush(stdout);
    }
    
    while(depth[0]=='2'){
        char playerID[2][100];
        scanf("%s%s", playerID[0], playerID[1]);
        if(atoi(playerID[0])==atoi(playerID[1]))
            break;

        char players[2][10][100];
        char scores[2][10][100];

        for(int a=0;a<2;a++){
            
            pid = fork();
            if(pid<0){
                fprintf(stderr, "fork err\n");
                return 0;
            }
            else if(pid==0){
                close(fdget[a][0]);
                dup2(fdget[a][1], 1);
                execlp("./player", "./player", "-n", playerID[a], (char *)0);
            }
            wait(NULL);
            
            for(int b=0;b<10;b++){
                read(fdget[a][0], players[a][b], strlen(playerID[a]));
                int idx1=0;
                while(idx1==0||scores[a][b][idx1-1]!='\n'){
                    read(fdget[a][0], &scores[a][b][idx1++], 1);
                }
                scores[a][b][idx1-1] = '\0';
            }
            
        }

        for(int a=0;a<10;a++){
            int lu = atoi(lucky_num);
            int s0 = atoi(scores[0][a]);
            int s1 = atoi(scores[1][a]);
            if(abs(s0-lu)<abs(s1-lu)||(abs(s0-lu)==abs(s1-lu)&&atoi(playerID[0])<atoi(playerID[1])))
                printf("%s %s\n", playerID[0], scores[0][a]);
            else
                printf("%s %s\n", playerID[1], scores[1][a]);
        }
        fflush(stdout);
    }

    return 0;
}

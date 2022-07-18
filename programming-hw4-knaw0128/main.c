#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<pthread.h>
#include<time.h>
#include<unistd.h>

FILE *input_file,*output_file;
int thread_number,col,row,epo;
int dx[8]={1,1,0,-1,-1,-1,0,1};
int dy[8]={0,-1,-1,-1,0,1,1,1};
int did[1000];
int size,now;
int **world[2];
int finish;

pthread_mutex_t lock;
pthread_cond_t cond_var;

typedef struct arg{
    int str_row;
    int str_col;
    int length;
    int now;
} arg;

void get_input(){
    char buf[128];
    fgets(buf,128,input_file);
    char *space,*tmp;
    space=strstr(buf," ");
    *space='\0';tmp=++space;
    row=atoi(buf);
    space=strstr(tmp," ");
    *space='\0';
    col=atoi(tmp);
    tmp=++space;
    epo=atoi(tmp);
    world[0]=(int **)malloc(sizeof(int*)*row);
    world[1]=(int **)malloc(sizeof(int*)*row);
    for(int a=0;a<row;a++){
        size+=col+5;
        world[0][a]=(int *)malloc(sizeof(int)*col+5);
        world[1][a]=(int *)malloc(sizeof(int)*col+5);
        char *tmp=(char *)malloc(sizeof(char)*(col+5));
        fgets(tmp,col+5,input_file);
        for(int b=0;b<col;b++){
            if(tmp[b]=='O')
                world[0][a][b]=1;
            else 
                world[0][a][b]=0;
        }
    }
}

int find_neighbor(int y,int x){
    int neighbor=0;
    for(int a=0;a<8;a++){
        int ny=y+dy[a];
        int nx=x+dx[a];
        if(ny>=0&&ny<row&&nx>=0&&nx<col)
            neighbor+=world[now][ny][nx];
    }
    return neighbor;
}

void one_generation(arg *data){
    int start=(data->str_row)*col+data->str_col;
    for(int a=start;a<start+data->length;a++){
        int cur_y=a/col;
        int cur_x=a%col;
        int neighbor=find_neighbor(cur_y,cur_x);
        if(world[now][cur_y][cur_x]==0){
            if(neighbor==3)
                world[1-now][cur_y][cur_x]=1;
            else
                world[1-now][cur_y][cur_x]=0;
        }
        else if(world[now][cur_y][cur_x]==1){
            if(neighbor==3||neighbor==2)
                world[1-now][cur_y][cur_x]=1;
            else
                world[1-now][cur_y][cur_x]=0;
        }
    }
}

void wait_other(){
    pthread_mutex_lock(&lock);
    finish++;
    if(finish<thread_number){
        pthread_cond_wait(&cond_var, &lock);
    }
    else{
        now=1-now;
        finish=0;
        pthread_cond_broadcast(&cond_var);
    }
    pthread_mutex_unlock(&lock);
}

void* run_thread(void* in){
    arg* data=(arg *)in;
    for(int a=0;a<epo;a++){
        one_generation(data);
        wait_other();
    }
    return NULL;
}

void thread_mode(){
    pthread_t tid[105];
    arg data[105];
    int all=row*col;
    int pre=all/thread_number;
    int left=all%thread_number;
    int current=0;
    pthread_cond_init(&cond_var,NULL);
    pthread_mutex_init(&lock,NULL);
    
    for(int a=0;a<thread_number;a++){
        data[a].str_row=current/col;
        data[a].str_col=current%col;
        if(left>0){
            data[a].length=pre+1;
            left--;
            current+=pre+1;
        }
        else{
            data[a].length=pre;
            current+=pre;
        }
        pthread_create(&tid[a],NULL,run_thread,(void *)&data[a]);
    }

    for(int a=0;a<thread_number;a++){
        pthread_join(tid[a],NULL);
    }
    pthread_cond_destroy(&cond_var);
    pthread_mutex_destroy(&lock);
}

// void process_mode(){
// }

void write_ans(){
    for(int a=0;a<row;a++){
        for(int b=0;b<col;b++){
            if(world[now][a][b]==1 )
                fwrite("O",1,1,output_file);
            else 
                fwrite(".",1,1,output_file);
        }
        if(a!=row-1)
            fwrite("\n",1,1,output_file);
    }
    fclose(output_file);
}

int main(int argc,char **argv){
    
    if(argc!=5){
        printf("wrong argument\n");
        return 0;
    }
    thread_number=atoi(argv[2]);
    input_file=fopen(argv[3], "r");
    output_file=fopen(argv[4], "w");
    get_input();
    fclose(input_file);

    if(strcmp(argv[1],"-t")==0)
        thread_mode();
    // else
    //     process_mode();

    write_ans();    

    return 0;
}
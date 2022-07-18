#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)

typedef struct {
    char hostname[512];  // server's hostname
    unsigned short port;  // port to listen
    int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
    char host[512];  // client's host
    int conn_fd;  // fd to talk with client
    char buf[512];  // data sent by/to client
    size_t buf_len;  // bytes used by buf
    // you don't need to change this.
    int id;
    int wait_for_write;  // used by handle_read to know if the header is read or not.
} request;

typedef struct {
    int id;          //902001-902020
    int AZ;          
    int BNT;         
    int Moderna;     
}registerRecord;

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list
registerRecord *all;
int doing[30];

static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance

int handle_read(request* reqP) {
    int r;
    char buf[512];

    // Read in request from client
    r = read(reqP->conn_fd, buf, sizeof(buf));
    if (r < 0) return -1;
    if (r == 0) return 0;
    char* p1 = strstr(buf, "\015\012");
    int newline_len = 2;
    if (p1 == NULL) {
       p1 = strstr(buf, "\012");
        if (p1 == NULL) {
            ERR_EXIT("this really should not happen...");
        }
    }
    size_t len = p1 - buf + 1;
    memmove(reqP->buf, buf, len);
    reqP->buf[len - 1] = '\0';
    reqP->buf_len = len-1;
    return 1;
}

int is_locked(int fd, int start, int whence){
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = start;
    lock.l_len = sizeof(registerRecord);
    lock.l_whence = whence;
    int trylock =  fcntl(fd, F_GETLK, &lock);
    if(trylock==-1){
        fprintf(stderr, "NONONONO   errno = %d\n",errno);
        return -1;
    }
    return (lock.l_type == F_UNLCK) ? 0 : 1;
}

int translate(int conn_fd){
    int ID = atoi(requestP[conn_fd].buf), idx;
    for(idx=0;idx<20;idx++)
        if(all[idx].id==ID)
            break;
    if (idx < 20 && strlen(requestP[conn_fd].buf) == 6)
        return idx;
    return -1;
}

void dolock(int fd, int start, int whence, int locktype){
    struct flock lock;
    lock.l_type = locktype;
    lock.l_start = start;
    lock.l_len = sizeof(registerRecord);
    lock.l_whence = whence;
    int trylock =  fcntl(fd, F_SETLK, &lock);
    if(trylock==-1){
        fprintf(stderr, "NONONONO  dododo  errno = %d\n",errno);
        perror("Error : ");
    }
}

int main(int argc, char **argv){

    // Parse args.
    if (argc != 2){
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        exit(1);
    }
    for(int a=0;a<30;a++)
        doing[a] = a;

    struct sockaddr_in cliaddr; // used by accept()
    int clilen;
    int conn_fd; // fd for a new connection with client
    int file_fd; // fd for file that we open for reading
    char buf[1024];
    int buf_len;
    all = malloc(sizeof(registerRecord) * 20);

    // Initialize server
    init_server((unsigned short)atoi(argv[1]));

    // Loop for handling connections
    fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);

    char buffer[1024];
    int DB = open("./registerRecord", O_RDWR| O_CREAT);

    struct timeval tw;
    tw.tv_sec = 0;
    tw.tv_usec = 50;
    fd_set ready;
    fd_set connect;

    int incomeFD[100]={0};
    for(int a=0;a<100;a++)
        incomeFD[a] = 0;
    int largefd=-1;

    while (1){
        // TODO: Add IO multiplexing
        read(DB, all, sizeof(registerRecord) * 20);
        // Check new connection
        FD_ZERO(&ready);
        FD_SET(svr.listen_fd, &ready);
        for(int a=0;a<100;a++)
            if(incomeFD[a])
                FD_SET(a, &ready);

        int co = select(maxfd+1, &ready, NULL, NULL, NULL);

        if(co&&FD_ISSET(svr.listen_fd, &ready)){
            clilen = sizeof(cliaddr);
            conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
            if (conn_fd < 0) {
                if (errno == EINTR || errno == EAGAIN) continue;  // try again
                if (errno == ENFILE) {
                    (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
                    continue;
                }
                ERR_EXIT("accept");
            }
            incomeFD[conn_fd] = 1;
            if(conn_fd>largefd)
                largefd = conn_fd;
            requestP[conn_fd].conn_fd = conn_fd;
            requestP[conn_fd].wait_for_write = -1;
            strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
            fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);

            char *greeting = "Please enter your id (to check your preference order):\n";
            write(conn_fd, greeting, strlen(greeting));
            continue;
        }
            
        for(conn_fd=4;conn_fd<largefd+1;conn_fd++){
            DB = open("./registerRecord", O_RDWR| O_CREAT);
            read(DB, all, sizeof(registerRecord) * 20);
            if(!FD_ISSET(conn_fd, &ready))
                continue;
            
            int ret = handle_read(&requestP[conn_fd]); // parse data from client to requestP[conn_fd].buf
            if (ret < 0){
                fprintf(stderr, "bad request from %s\n", requestP[conn_fd].host);
                continue;
            }

            // TODO: handle requests from clients
            int ID = translate(conn_fd);
            if (ID>=0){
                char *vaccine[3];
                vaccine[all[ID].BNT - 1] = "BNT";
                vaccine[all[ID].AZ - 1] = "AZ";
                vaccine[all[ID].Moderna - 1] = "Moderna";
                
#ifdef READ_SERVER
                sprintf(buf, "Your preference order is %s > %s > %s.\n", vaccine[0], vaccine[1], vaccine[2]);
#elif defined WRITE_SERVER
                sprintf(buf, "Your preference order is %s > %s > %s.\nPlease input your preference order respectively(AZ,BNT,Moderna):\n", vaccine[0], vaccine[1], vaccine[2]);
#endif
                if(is_locked(DB, ID*sizeof(registerRecord), SEEK_SET)||doing[ID]!=ID){
                    sprintf(buf, "Locked.\n");
                    write(requestP[conn_fd].conn_fd, buf, strlen(buf)); 
                    incomeFD[requestP[conn_fd].conn_fd] = 0;
                    close(requestP[conn_fd].conn_fd);
                    free_request(&requestP[conn_fd]);
                    continue;
                }
                else{
                    doing[ID] = -1;
#ifdef READ_SERVER
                    dolock(DB, ID*sizeof(registerRecord), SEEK_SET, F_RDLCK);
#elif defined WRITE_SERVER
                    dolock(DB, ID*sizeof(registerRecord), SEEK_SET, F_WRLCK);
#endif                    
                }
            }
            else{
                sprintf(buf, "[Error] Operation failed. Please try again.\n");
            }

#ifdef READ_SERVER
            write(requestP[conn_fd].conn_fd, buf, strlen(buf)); 

#elif defined WRITE_SERVER                    
            if(requestP[conn_fd].wait_for_write == -1){
                write(requestP[conn_fd].conn_fd, buf, strlen(buf));
                if(ID>=0){
                    requestP[conn_fd].wait_for_write = ID;
                    continue;
                }
            }
            else{
                int error = 0;
                int order[3], oidx =0;
        //checking
                for(int a=0;a<requestP[conn_fd].buf_len;a++){
                    int input = requestP[conn_fd].buf[a]-'1'+1;
                    if(a>4||(a%2==0&&(input>3||input<1))||(a%2==1&&requestP[conn_fd].buf[a]!=' ')){
                        error = 1;
                        break;
                    }
                    else if(a%2==0)
                        order[oidx++] = input;
                }
                int check =0;
                for(int a=0;a<3;a++)
                    check+=order[a];
                if(check!=6)
                    error = 1;

                if(error){
                    sprintf(buf,"[Error] Operation failed. Please try again.\n");
                    write(requestP[conn_fd].conn_fd, buf, strlen(buf));
                    
                    doing[requestP[conn_fd].wait_for_write] = requestP[conn_fd].wait_for_write;
                    dolock(DB, requestP[conn_fd].wait_for_write*sizeof(registerRecord), SEEK_SET, F_UNLCK);
                    requestP[conn_fd].wait_for_write = -1;
                    incomeFD[requestP[conn_fd].conn_fd] = 0;

                    close(requestP[conn_fd].conn_fd);
                    free_request(&requestP[conn_fd]);
                    continue;
                }
                else{
                    all[requestP[conn_fd].wait_for_write].AZ = order[0];
                    all[requestP[conn_fd].wait_for_write].BNT = order[1];
                    all[requestP[conn_fd].wait_for_write].Moderna = order[2];
                    char *vaccine[3];
                    vaccine[all[requestP[conn_fd].wait_for_write].BNT-1] = "BNT";
                    vaccine[all[requestP[conn_fd].wait_for_write].AZ-1] = "AZ";
                    vaccine[all[requestP[conn_fd].wait_for_write].Moderna-1] = "Moderna";
                    sprintf(buf,"Preference order for %d modified successed, new preference order is %s > %s > %s.\n",902001+requestP[conn_fd].wait_for_write, vaccine[0], vaccine[1], vaccine[2]);   
                }
            }

            
#endif
            
            incomeFD[requestP[conn_fd].conn_fd] = 0;
#ifdef READ_SERVER
            doing[ID] = ID;
            dolock(DB, ID*sizeof(registerRecord), SEEK_SET, F_UNLCK);
#elif defined WRITE_SERVER
            doing[requestP[conn_fd].wait_for_write] = requestP[conn_fd].wait_for_write;
            dolock(DB, requestP[conn_fd].wait_for_write*sizeof(registerRecord), SEEK_SET, F_UNLCK);
            requestP[conn_fd].wait_for_write = -1;
#endif 
            write(requestP[conn_fd].conn_fd, buf, strlen(buf));
            close(requestP[conn_fd].conn_fd);
            free_request(&requestP[conn_fd]);
            pwrite(DB, all, 20*sizeof(registerRecord), SEEK_SET);
            
        }
    
    }
    free(requestP);
    return 0;
}

// ======================================================================================================
// You don't need to know how the following codes are working
#include <fcntl.h>

static void init_request(request *reqP){
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->id = 0;
}

static void free_request(request *reqP){
    /*if (reqP->filename != NULL) {
        free(reqP->filename);
        reqP->filename = NULL;
    }*/
    init_request(reqP);
}

static void init_server(unsigned short port){
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0)
        ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&tmp, sizeof(tmp)) < 0){
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0){
        ERR_EXIT("listen");
    }

    // Get file descripter table size and initialize request table
    maxfd = getdtablesize();
    requestP = (request *)malloc(sizeof(request) * maxfd);
    if (requestP == NULL){
        ERR_EXIT("out of memory allocating all requests");
    }
    for (int i = 0; i < maxfd; i++){
        init_request(&requestP[i]);
    }
    requestP[svr.listen_fd].conn_fd = svr.listen_fd;
    strcpy(requestP[svr.listen_fd].host, svr.hostname);

    return;
}

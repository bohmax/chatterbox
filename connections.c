#include <connections.h>

int openConnection(char* path, unsigned int ntimes, unsigned int secs)
{
    struct sockaddr_un serv_addr;
    int sockfd;
    int ris=0;
    SYSCALL(sockfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket");
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path,path, strlen(path)+1);
    int notused,nbites;
    for(int i=ntimes;i>0;i--){
        if((notused=connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))!=-1)){
            if((nbites=readn(sockfd, &ris, sizeof(int)))>0)
                if(ris==OP_OK)
                    return sockfd;
        }
        sleep(secs);
    }
    return -1;
}

int readHeader(long connfd, message_hdr_t *hdr)
{
    int i=(int)readn((int)connfd, hdr, sizeof(message_hdr_t));
    return i;
}

int readData(long fd, message_data_t *data)
{
    int i;
    i=(int)readn((int)fd, &data->hdr, sizeof(message_data_hdr_t));
    if (i==-1) {
        return i;
    }
    if(data->hdr.len>0){
        if(data->buf)
            data->buf=NULL;
        ERRORSYSHEANDLER(data->buf,malloc(sizeof(char)*data->hdr.len),NULL,"malloca data")
        i=(int)readn((int)fd, data->buf, data->hdr.len);
    }
    return i;
}

int readMsg(long fd, message_t *msg)
{
    int i;
    i=(int)readn((int)fd, &msg->hdr, sizeof(message_hdr_t));
    if(i==-1){
        return i;
    }
    switch (msg->hdr.op) {
        case REGISTER_OP: case CONNECT_OP: case USRLIST_OP: case DISCONNECT_OP: case GETPREVMSGS_OP:{}break;
        default:
        //case POSTTXT_OP:case POSTTXTALL_OP:case POSTFILE_OP:case GETFILE_OP:case TXT_MESSAGE:case FILE_MESSAGE: case CREATEGROUP_OP:
            i=(int)readn((int)fd, &msg->data.hdr, sizeof(message_data_hdr_t));
            if(i==-1){
                return i;
            }
            if(msg->data.hdr.len>0){
                if(msg->data.buf)
                    msg->data.buf=NULL;
                ERRORSYSHEANDLER(msg->data.buf,malloc(sizeof(char)*msg->data.hdr.len),NULL,"malloca data")
                i=(int)readn((int)fd, msg->data.buf, msg->data.hdr.len);
            break;
        }
    }
    return i;
}

int sendRequest(long fd, message_t *msg){
    int err;
    err=(int)writen((int)fd,&msg->hdr,sizeof(message_hdr_t));
    if(strcmp(msg->data.hdr.receiver,"")!=0 || msg->data.hdr.len>0){
        err=(int)writen((int)fd,&msg->data.hdr,sizeof(message_data_hdr_t));
        if(msg->data.hdr.len>0){ //se c'è qualcosa su buf spezzo il messaggio e invio
            err=(int)writen((int)fd,msg->data.buf,msg->data.hdr.len);
        }
    }
    return err;
}

int sendData(long fd, message_data_t *msg)
{
    int err;
    err=(int)writen((int)fd, &msg->hdr, sizeof(message_data_hdr_t));
    if(err==-1){
        return -1;
    }
    err=(int)writen((int)fd, msg->buf,msg->hdr.len);
    return err;
}

int sendHeader(long fd, message_hdr_t *hdr)
{
    int i=(int)writen((int)fd, hdr, sizeof(message_hdr_t));
    return i;
}

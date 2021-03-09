#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <sys/time.h>
using namespace std;

struct activeGames
{
    struct timeval time;
    int index;
    int sockNr1;
    int sockNr2;
    int timeTaken1;
    int timeTaken2;
    bool socket1Ready;
    bool socket2Ready;
    bool concluded;
    bool started;
};

activeGames games[10];

void fill()
{
    for(int i = 0; i < 10; i++)
    {
        games[i].started = false;
        games[i].index = -1;
        games[i].socket1Ready = false;
        games[i].socket2Ready = false;
        games[i].sockNr1 = -1;
        games[i].sockNr2 = -1;
        games[i].concluded = false;
    }
   
}
void sendTimingMsg(int arrayIndex, int sec)
{
    bool isFinished;
    char timeMsg[50];
    memset(timeMsg,0,sizeof(timeMsg));
    int sendValue = 0;
    int sendValue2 = 0;
    sprintf(timeMsg,"TIME %d\n",sec);                            
    sendValue = send(games[arrayIndex].sockNr1,timeMsg,strlen(timeMsg),0);
    if(sendValue < 0)
    {
        printf("Error sending hello msg\n");                                    
        exit(3);
    }
    printf("sendbytes1: %d\n", sendValue);
    fflush(stdout);
    sendValue2 = send(games[arrayIndex].sockNr2,timeMsg,strlen(timeMsg),0);
    if(sendValue < 0)
    {
        printf("Error sending hello msg\n");                                    
        exit(3);
    }
    printf("sendbytes2: %d\n", sendValue2);
    printf("efter båda skickas\n");
    
    fflush(stdout);
}

int main(int argc, char *argv[])
{

    if(argc!=2)
    {
        printf("To few arguments!\n");
        exit(1);
    }

    fill();
    char delim[]=":";
    char *Desthost=strtok(argv[1],delim);
    char *Destport=strtok(NULL,delim);


    struct sockaddr_storage clientAddr;
    socklen_t addrLenght;

    struct addrinfo hints, *p, *servinfo;

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
   

    fd_set master;
    fd_set read_fds;
    int maxFds;
    int listenSocket;
    int acceptFd;
    int nrOfPlayers = 0;
    int gameCounter = 0;

    memset(&hints,0,sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;

    char buf[256];
    char menuMsg[] = "MENU\n";
    char waitingForPlayer[] = "WAIT\n";
    char gameStartingMsg[] = "START\n";
    char ready[] = "READY";
    char operation[256];

    memset(buf,0,sizeof(buf));
    
    int yes = 1;
    int recivedValue;
    int sendValue;
    int choice = 0;
    int queueIndex[10][2];

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    recivedValue = getaddrinfo(Desthost,Destport,&hints,&servinfo);
    if(recivedValue !=0)
    {
        printf("getaddrInfo error\n");
        exit(1);
    }
        

    for(p = servinfo; p != NULL; p->ai_next)
    {
        listenSocket = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
        if(listenSocket < 0)
        {
            continue;
        }

        setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int));

        if(bind(listenSocket,p->ai_addr,p->ai_addrlen) < 0)
        {
            close(listenSocket);
            continue;
        }
        break;
    }

    if(p == NULL)
    {
        printf("failed to bind\n");
        exit(1);
    }

    freeaddrinfo(servinfo);

    if(listen(listenSocket,20) < 0)
    {
        printf("listen error\n");
        exit(3);
    }


    FD_SET(listenSocket,&master);
    
    maxFds = listenSocket;
    #ifdef DEBUG
    printf("Before loop\n");
    #endif
    while(1)
    {
        memset(buf,0,sizeof(buf));
        memset(operation,0,sizeof(operation));
        read_fds = master;
        if(select(maxFds+1,&read_fds,NULL,NULL,&timeout) == -1)
        {
            printf("Select error\n");
            exit(2);
        }
        


        for(int i = 0; i <= maxFds; i++)
        {
            if(FD_ISSET(i,&read_fds))// Got connecion
            {
                if(i == listenSocket)
                {
                    addrLenght = sizeof(struct sockaddr_storage);
                    acceptFd =  accept(listenSocket,(struct sockaddr *)&clientAddr,&addrLenght);
                    if(acceptFd == -1)
                    {
                        printf("accept Error\n");
                    }
                    else
                    {
                        FD_SET(acceptFd,&master);
                        if(acceptFd>maxFds)
                        {
                            maxFds = acceptFd;
                        }

                        #ifdef DEBUG
                        printf("new Connection\n");
                        #endif
                        sendValue = send(acceptFd,menuMsg,sizeof(menuMsg),0);
                        if(sendValue < 0)
                        {
                            printf("Error sending hello msg\n");
                            //close(acceptFd);
                            break;
                        }
                        #ifdef DEBUG
                        printf("Hello msg sent\n");
                        #endif
                    }
                    continue;
                }
                else
                {
                    //handle data from client
                    recivedValue = recv(i,buf,sizeof(buf),0);

                    if(recivedValue <= 0)
                    {
                        if(recivedValue == 0)
                        {
                            close(i);
                            FD_CLR(i,&master);
                        
                        }
                        else if(recivedValue == -1)
                        {
                            close(i);
                            FD_CLR(i,&master);
                        }

                    }
                    printf("recived msg: %s",buf);
                    sscanf(buf,"%s",operation);

                    if(strcmp(operation, "MENU") == 0)
                    {
                        sscanf(buf,"%s %d",operation,&choice);
                        if(choice == 1)
                        {

                            
                            queueIndex[gameCounter][nrOfPlayers] = i;                            
                            nrOfPlayers++;
                            if((nrOfPlayers % 2) == 0)
                            {
                                //Start game between the two index nummbers found in queueIndex and then gameCounter++
                                games[gameCounter].sockNr2 = i;
                                printf("Game Ready to start\n");
                                for(int j = 0; j < 2; j++)
                                {
                                    sendValue = send(queueIndex[gameCounter][j],gameStartingMsg,strlen(gameStartingMsg),0);
                                    if(sendValue < 0)
                                    {
                                        printf("Error sending hello msg\n");
                                        //close(acceptFd);
                                        break;
                                    }
                                    #ifdef DEBUG
                                    printf("Queuing players = %d", nrOfPlayers);
                                    #endif
                                }
                                games[gameCounter].index = gameCounter;                             
                                gameCounter++;                            
                                
                            }
                            else
                            {        
                                games[gameCounter].sockNr1 = i;                    
                                sendValue = send(i,waitingForPlayer,strlen(waitingForPlayer),0);
                                if(sendValue < 0)
                                {
                                    printf("Error sending hello msg\n");
                                    //close(acceptFd);
                                    break;
                                }
                                #ifdef DEBUG
                                printf("Queuing players = %d", nrOfPlayers);
                                #endif
                            }
                        }
                        else if(choice == 2)
                        {
                            //WATCH
                            printf("Watch not made yet stopid\n");                        
                        }
                        else
                        {
                            //Should not get here klient wont let it through until 1 or 2
                            printf("Bad nr\n");
                        }
                    }
                    else if(strcmp(operation, "STOP") == 0)
                    {
                        nrOfPlayers--;
                        printf("Player left queue\n"); 
                        sendValue = send(i,menuMsg,strlen(menuMsg),0);
                        if(sendValue < 0)
                        {
                            printf("Error sending hello msg\n");
                            //close(acceptFd);
                            break;
                        }                        
                    }
                    else if(strcmp(operation, "READY") == 0)
                    {
                        printf("Ready recived\n");
                        for(int j = 0; j < gameCounter; j++)
                        {
                            if(games[j].sockNr2 == i)
                            {
                                games[j].socket2Ready = true;
                                /*
                                sendValue = send(i,ready,strlen(ready),0);
                                if(sendValue < 0)
                                {
                                    printf("Error sending hello msg\n");
                                    //close(acceptFd);
                                    break;
                                }
                                */
                            }
                            if(games[j].sockNr1 == i)
                            {
                                games[j].socket1Ready = true;
                                /*
                                sendValue = send(i,ready,strlen(ready),0);
                                if(sendValue < 0)
                                {
                                    printf("Error sending hello msg\n");
                                    //close(acceptFd);
                                    break;
                                }                            
                                */
                            }                                                       
                        }
                        for(int j = 0; j < gameCounter; j++)
                        {
                            if(games[j].socket1Ready == true && games[j].socket2Ready == true && games[j].started != true)
                            {
                                // STarta spelet
                                printf("Båda klienter redo!\n");  
                                games[j].started = true;                                                            
                                sendTimingMsg(j,3);
                                gettimeofday(&games[j].time,NULL);
                                fflush(stdout);      
                            }
                        }
                       
                    }
                }
            }
            else
            {
                for(int i = 0; i < gameCounter; i++)
                {
                    struct timeval comparetime;
                    gettimeofday(&comparetime,NULL);
                    if((comparetime.tv_sec - games[i].time.tv_sec) > 1 && games[i].started == true)
                    {                
                        sendTimingMsg(i,2);
                    }
                }
            }
        }
      
    }


    return 0;
}
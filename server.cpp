#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
using namespace std;


int main(int argc, char *argv[])
{

    if(argc!=2)
    {
        printf("To few arguments!\n");
        exit(1);
    }

    char delim[]=":";
    char *Desthost=strtok(argv[1],delim);
    char *Destport=strtok(NULL,delim);


    struct sockaddr_storage clientAddr;
    socklen_t addrLenght;

    struct addrinfo hints, *p, *servinfo;

    fd_set master;
    fd_set read_fds;
    int maxFds;
    int listenSocket;
    int acceptFd;
    int nrOfPlayers = 0;
    int activeGames = 0;

    memset(&hints,0,sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;

    char buf[256];
    char menuMsg[] = "Please select:\n1.Play\n2.Watch\n0.Exit\n";
    char waitingForPlayer[] = "One more player requierd to start game\n";
    char gameStartingMsg[] = "Game startig in X\n";
    char operation[256];

    memset(buf,0,sizeof(buf));
    
    int yes = 1;
    int recivedValue;
    int sendValue;
    int choice = 0;
    int queueIndex[3][2];

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
        if(select(maxFds+1,&read_fds,NULL,NULL,NULL) == -1)
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
                            
                            
                            queueIndex[activeGames][nrOfPlayers] = i;
                            
                            nrOfPlayers++;
                            if(nrOfPlayers == 2)
                            {
                                //Start game between the two index nummbers found in queueIndex and then activegames++
                                printf("Game Ready to start\n");
                                for(int j = 0; j < 2; j++)
                                {
                                    sendValue = send(queueIndex[activeGames][j],gameStartingMsg,sizeof(gameStartingMsg),0);
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
                                activeGames++;
                            }
                            else
                            {                            
                                sendValue = send(i,waitingForPlayer,sizeof(waitingForPlayer),0);
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
                        sendValue = send(i,menuMsg,sizeof(menuMsg),0);
                        if(sendValue < 0)
                        {
                            printf("Error sending hello msg\n");
                            //close(acceptFd);
                            break;
                        }
                    }
                    else
                    {

                    }
                }
            }
        }
    }


    return 0;
}
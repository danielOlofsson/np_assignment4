#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/unistd.h>
#include <iostream>
#define DEBUG

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

    struct addrinfo hints, *serverinfo, *servaddr;
    fd_set readfds;
    int recivedValue = 0;
    int sendValue  = 0;
    int clientSocket;
    int choice = -1;
    bool isSent = false;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_PASSIVE; 

    if ((recivedValue = getaddrinfo(Desthost, Destport, &hints, &serverinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(recivedValue));
        exit(2);
    }

    for(servaddr = serverinfo; servaddr != NULL; servaddr = servaddr->ai_next) 
    {
        if ((clientSocket = socket(servaddr->ai_family, servaddr->ai_socktype, servaddr->ai_protocol)) == -1)
        {
            perror("talker: socket");
            continue;
        }
        break;
    }

    if (servaddr == NULL) 
    {
        fprintf(stderr, "talker: failed to create socket\n");
        exit(3);
    }

    if (connect(clientSocket,servaddr->ai_addr, servaddr->ai_addrlen) < 0 ) 
    {
        perror("connecton error .\n");
        exit(1);
    }
    
    char buf[256];
    char sendMsg[256];
    char inputMsg[256];
    char stopQ[] = "STOP\n";

    fd_set masterFds;
    FD_ZERO(&readfds);
    FD_ZERO(&masterFds);
    FD_SET(STDIN_FILENO,&masterFds);
    FD_SET(clientSocket, &masterFds);

    while(1)
    {
        readfds = masterFds;
        memset(buf,0,sizeof(buf));
        memset(sendMsg,0,sizeof(sendMsg));
        
        recivedValue = select(clientSocket +1, &readfds,NULL,NULL,NULL);
        if(recivedValue == -1)
        {
            printf("Error with select");
            continue;
        }

        if(FD_ISSET(clientSocket, &readfds))
        {   
            recivedValue = recv(clientSocket, buf, sizeof(buf), 0);
            if (recivedValue == -1) 
            {
                perror("sendto:");
                exit(1);
            }
            else if(recivedValue == 0)
            {
                close(clientSocket);
                break;
            }
            //printf("Message recived: %s",buf);

            if(strcmp(buf,"Please select:\n1.Play\n2.Watch\n0.Exit\n") == 0)
            {
                //Menu text recived
                
                while(isSent == false)
                {                                    
                    printf("%s",buf);                    
                    cin >> choice;
                    cin.ignore();
                    
                    if(choice == 1 || choice == 2)
                    {
                        sprintf(sendMsg,"MENU %d\n",choice);
                        sendValue = send(clientSocket, sendMsg, strlen(sendMsg), 0);
                        if (sendValue == -1) 
                        {
                            perror("sendto:");
                            break;
                        }  
                        isSent = true;                       
                    }
                    else if(choice == 0)
                    {
                        close(clientSocket);
                        exit(3);
                    }
                    else
                    {
                        printf("choose from given options!\n");                                                
                    }                    
                }
                isSent = false;
            }
            else if(strcmp(buf,"One more player requierd to start game\n") == 0)
            {
                printf("Type 'EXIT' to leave\n");
                while(1)
                {
                    if(FD_ISSET(clientSocket, &readfds))
                    {
                        break;
                    }
                    memset(inputMsg,0,sizeof(inputMsg));
                    fgets(inputMsg, 256, stdin);

                    if(strcmp(inputMsg,"EXIT\n") == 0)
                    {
                        //send msg that you left the queue
                        sendValue = send(clientSocket, stopQ, strlen(stopQ), 0);
                        if (sendValue == -1) 
                        {
                            perror("sendto:");
                            exit(4);
                        }
                        printf("Sent stop Queue msg\n");
                        break;
                    }                
                }        
            }
            else if(strcmp(buf,"Game startig in X\n") == 0)
            {
                printf("buf:\n%s",buf);
            }          
        }
    }


    return 0;
}

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
#include <sys/time.h>
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
    char ready[] = "READY\n";
    char command[50];
    bool startSent = false;
    int sec = 0;
    int round = 0;
    int score1 = 0;
    int score2 = 0;

    fd_set masterFds;
    FD_ZERO(&readfds);
    FD_ZERO(&masterFds);
    FD_SET(STDIN_FILENO,&masterFds);
    FD_SET(clientSocket, &masterFds);

    while(1)
    {
        readfds = masterFds;
        FD_SET(STDIN_FILENO,&masterFds);
        memset(buf,0,sizeof(buf));
        memset(sendMsg,0,sizeof(sendMsg));
        
        recivedValue = select(clientSocket +1, &readfds,NULL,NULL,NULL);
        if(recivedValue == -1)
        {
            printf("Error with select");
            continue;
        }
        if(FD_ISSET(STDIN_FILENO,&readfds))
        {           
            
            if(strcmp(command,"MENU") == 0)
            {    
                memset(inputMsg,0,sizeof(inputMsg));
                fgets(inputMsg, 256, stdin);
                
               
                if(strcmp(inputMsg,"1\n") == 0 || strcmp(inputMsg,"2\n") == 0 || strcmp(inputMsg,"0\n") == 0)
                {
                    if(strcmp(inputMsg, "1\n") == 0)
                    {
                        sprintf(sendMsg,"MENU %d\n",1);
                        sendValue = send(clientSocket, sendMsg, strlen(sendMsg), 0);
                        if (sendValue == -1) 
                        {
                            perror("sendto:");
                            break;
                        }                        
                    }
                    else if(strcmp(inputMsg,"2\n") == 0)
                    {
                        sprintf(sendMsg,"MENU %d\n",2);
                        sendValue = send(clientSocket, sendMsg, strlen(sendMsg), 0);
                        if (sendValue == -1) 
                        {
                            perror("sendto:");
                            break;
                            
                        }
                    }
                    else if(strcmp(inputMsg,"0\n") == 0)
                    {
                        close(clientSocket);
                        exit(3);
                    }
                    
                }
                else
                {
                    printf("Choose between given nummbers!\n");
                    printf("1.Play\n2.Watch\n0.Exit\n");
                    
                }
                               
            }
            else if(strcmp(command, "WAIT") == 0)
            {       
                memset(inputMsg,0,sizeof(inputMsg));
                fgets(inputMsg, 256, stdin);                         
                if(strcmp(inputMsg,"\n") == 0)
                {
                    //send msg that you left the queue
                    sendValue = send(clientSocket, stopQ, strlen(stopQ), 0);
                    
                    if (sendValue == -1) 
                    {
                        perror("sendto:");
                        exit(4);
                    }
                    printf("Sent stop Queue msg\n");                                    
                }

            }
            else if(strcmp(command, "START") == 0)
            {
                
                memset(inputMsg,0,sizeof(inputMsg));
                fgets(inputMsg, 256, stdin);                         
                if(strcmp(inputMsg,"\n") == 0)
                {
                    //send msg that you left the queue
                    sendValue = send(clientSocket, ready, strlen(ready), 0);
                    
                    if (sendValue == -1) 
                    {
                        perror("sendto:");
                        exit(4);
                    }
                    printf("Sent ready msg\n");
                    //startSent = true;                                    
                }
            }
            else if(strcmp(command, "ROUND") == 0)
            {
                memset(inputMsg,0,sizeof(inputMsg));
                fgets(inputMsg, 256, stdin);
                
               
                if(strcmp(inputMsg,"1\n") == 0 || strcmp(inputMsg,"2\n") == 0 || strcmp(inputMsg,"3\n") == 0)
                {
                    if(strcmp(inputMsg, "1\n") == 0)
                    {
                        sprintf(sendMsg,"ROUND %d\n",1);
                        sendValue = send(clientSocket, sendMsg, strlen(sendMsg), 0);
                        if (sendValue == -1) 
                        {
                            perror("sendto:");
                            break;
                        } 
                    }
                    else if(strcmp(inputMsg, "2\n") == 0)
                    {
                        sprintf(sendMsg,"ROUND %d\n",2);
                        sendValue = send(clientSocket, sendMsg, strlen(sendMsg), 0);
                        if (sendValue == -1) 
                        {
                            perror("sendto:");
                            break;
                        } 
                    }
                    else if(strcmp(inputMsg, "3\n") == 0)
                    {
                        sprintf(sendMsg,"ROUND %d\n",3);
                        sendValue = send(clientSocket, sendMsg, strlen(sendMsg), 0);
                        if (sendValue == -1) 
                        {
                            perror("sendto:");
                            break;
                        }
                    }
                }
                else
                {
                    printf("Choose between given nummbers!\n");
                    printf("Select your option:\n1.Rock\n2.Paper\n3.Scissor\n");
                }
            }
            fflush(stdin);
            FD_CLR(STDIN_FILENO,&readfds);
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
            sscanf(buf,"%s",command);
            if(strcmp(command,"MENU") == 0)
            {
                printf("1.Play\n2.Watch\n0.Exit\n");
                fflush(stdout);                          
            }
            else if(strcmp(command,"WAIT") == 0)
            {
                printf("Waiting for another player, ENTER to stop\n");      
                fflush(stdout);                         
            }
            else if(strcmp(command,"START") == 0)
            {
                printf("Game is ready, press enter to accept\n\n");
                fflush(stdout);                             
            }
            else if(strcmp(command,"READY") == 0)
            {
                printf("You are now ready\n");
                fflush(stdout);
            }
            else if(strcmp(command, "TIME") == 0)
            {
                sscanf(buf,"%s %d",command, &sec);
                printf("Game is starting in %d seconds\n",sec);
                fflush(stdout);

            }
            else if(strcmp(command, "ROUND") == 0)
            {
                sscanf(buf,"%s %d",command, &round);
                printf("Round %d\n",round);
                printf("Select your option:\n1.Rock\n2.Paper\n3.Scissor\n");
                
                fflush(stdout);
            }
            else if(strcmp(command, "ROUNDF") == 0)
            {
                sscanf(buf,"%s %d %d",command, &score1, &score2);
                printf("Score %d - %d\n",score1, score2);
                fflush(stdout);
            }
            else if(strcmp(command, "WIN"))
            {
                sscanf(buf,"%s %d %d",command, &score1, &score2);
                printf("You Win!\nScore: %d - %d\n\n", score1, score2);
                fflush(stdout);
                memset(command,0,sizeof(command));
                printf("1.Play\n2.Watch\n0.Exit\n");
                fflush(stdout);
                sprintf(command, "MENU");
                
                
            }
            else if(strcmp(command, "LOSE"))
            {
                sscanf(buf,"%s %d %d",command, &score1, &score2);
                printf("You Lose..\nScore: %d - %d\n\n", score1, score2);                
                printf("1.Play\n2.Watch\n0.Exit\n");
                fflush(stdout);
                memset(command,0,sizeof(command));
                fflush(stdout);
                sprintf(command, "MENU");                
                
                
            }
            FD_CLR(clientSocket,&readfds);
        }
    }


    return 0;
}

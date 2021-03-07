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
    int recivedValue;
    int clientSocket;
    int choice =-1;

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

    fd_set masterFds;
    FD_ZERO(&readfds);
    FD_ZERO(&masterFds);
    FD_SET(STDIN_FILENO,&masterFds);
    FD_SET(clientSocket, &masterFds);

    while(1)
    {
        readfds = masterFds;
        memset(buf,0,sizeof(buf));
        
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
            printf("Message recived: %s",buf);

            if(strcmp(buf,"Please select:\n1.Play\n2.Watch\n0.Exit\n") == 0)
            {
                //Menu text recived
                printf("%s!",buf);
                
                cin >> choice;

                if(choice == 1)
                {
                    printf("Play\n");
                }
                else if(choice == 2)
                {
                    printf("Watch\n");
                }
                else if(choice == 0)
                {
                    printf("Exiting\n");
                    close(clientSocket);
                    break;
                }
                else
                {
                    printf("choose from given options!\n");
                }
            }
        }

        if(FD_ISSET(STDIN_FILENO,&readfds))
        {
            printf("Stop typing!\n");
        }
    }


    return 0;
}

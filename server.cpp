#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <sys/time.h>
#include <cstring>
using namespace std;

struct activeGames
{
    struct timeval time;
    int index;
    int sockNr1;
    int sockNr2;
    int timeTaken1;
    int timeTaken2;
    int score1;
    int score2;
    bool socket1Ready;
    bool socket2Ready;
    bool concluded;
    bool started;
    int secondsToCount;
    int roundNr;
    bool isAnswering;
    int choice1;
    int choice2;
    bool bothAnswered;
    int watching[100];
    int nrOfWatching;
    double sockTime1;
    double sockTime2;
    
};

activeGames games[100];

struct highscore
{
    int winnerScore;
    int looseScore;
    struct timeval deltaTimeWinner;
    struct timeval deltaTimeLooser;
    
};

struct highscore scoreList[100];

void fill()
{
    for(int i = 0; i < 100; i++)
    {
        games[i].started = false;
        games[i].index = -1;
        games[i].socket1Ready = false;
        games[i].socket2Ready = false;
        games[i].sockNr1 = -1;
        games[i].sockNr2 = -1;
        games[i].concluded = false;
        games[i].secondsToCount = 3;
        games[i].roundNr = 0;
        games[i].isAnswering = false;
        games[i].choice1 = 0;
        games[i].choice2 = 0;
        games[i].bothAnswered = false;
        games[i].nrOfWatching = 0;       
        for(int j = 0; j < 100; j++)
        {
            games[i].watching[j] = -1;
            
        }
    }   
}

void initializeScore()
{
    for(int i = 0; i < 100; i++)
    {
        scoreList[i].looseScore = 0;
        scoreList[i].winnerScore = 0;
    }
}

void sendTimingMsg(int arrayIndex)
{    
    char timeMsg[50];
    char roundMsg[50];
    memset(timeMsg,0,sizeof(timeMsg));
    gettimeofday(&games[arrayIndex].time,NULL);
    int sendValue = 0;
    int sendValue2 = 0;
    
    printf("seconds to count = %d\n",games[arrayIndex].secondsToCount);
    if(games[arrayIndex].secondsToCount == 0)
    {
        games[arrayIndex].roundNr++;
        sprintf(roundMsg,"ROUND %d\n",games[arrayIndex].roundNr);
        sendValue = send(games[arrayIndex].sockNr1,roundMsg,strlen(roundMsg),0);
        if(sendValue < 0)
        {
            printf("Error sending hello msg\n");                                    
            exit(3);
        }
        
        fflush(stdout);
        sendValue2 = send(games[arrayIndex].sockNr2,roundMsg,strlen(roundMsg),0);
        if(sendValue < 0)
        {
            printf("Error sending hello msg\n");                                    
            exit(3);
        }        
        printf("efter båda skickas\n");
        fflush(stdout);
        games[arrayIndex].isAnswering = true;
        for(int i = 0; i < games[arrayIndex].nrOfWatching; i++)
        {
                                       
            sendValue = send(games[arrayIndex].watching[i],roundMsg,strlen(roundMsg),0);
            if(sendValue < 0)
            {
                printf("Error sending hello msg\n");                                    
                exit(4);
            }
            printf("sendbytes1: %d\n", sendValue);
            fflush(stdout);
        }
    }
    else if(games[arrayIndex].secondsToCount > 0)
    {
        sprintf(timeMsg,"TIME %d\n",games[arrayIndex].secondsToCount);                            
        sendValue = send(games[arrayIndex].sockNr1,timeMsg,strlen(timeMsg),0);
        if(sendValue < 0)
        {
            printf("Error sending hello msg\n");                                    
            exit(4);
        }
        printf("sendbytes1: %d\n", sendValue);
        fflush(stdout);
        sendValue2 = send(games[arrayIndex].sockNr2,timeMsg,strlen(timeMsg),0);
        if(sendValue < 0)
        {
            printf("Error sending hello msg\n");                                    
            exit(4);
        }
        printf("sendbytes2: %d\n", sendValue2);
        printf("efter båda skickas\n");
        
        fflush(stdout);

        for(int i = 0; i < games[arrayIndex].nrOfWatching; i++)
        {
            sprintf(timeMsg,"TIME %d\n",games[arrayIndex].secondsToCount);                            
            sendValue = send(games[arrayIndex].watching[i],timeMsg,strlen(timeMsg),0);
            if(sendValue < 0)
            {
                printf("Error sending hello msg\n");                                    
                exit(4);
            }
            printf("sendbytes1: %d\n", sendValue);
            fflush(stdout);
        }
        games[arrayIndex].secondsToCount--;
    }
    
    
}

int rockPapperScissors(int index)
{
    int winner = 0;
    // Both have answerd
    // 1 = rock
    // 2 = paper
    // 3 = scissors
    // 4 = timeout
    if(games[index].choice1 == games[index].choice2)
    {
        //draw
        winner = 0;
    }
    else if(games[index].choice1 == 1 && games[index].choice2 == 2)
    {
        //sock 2 victory
        winner = 2;
    }
    else if(games[index].choice1 == 1 && games[index].choice2 == 3)
    {
        //sock 1 victory
        winner = 1;
    }
    else if(games[index].choice1 == 2 && games[index].choice2 == 1)
    {
        //sock 1 victory
        winner = 1;
    }
    else if(games[index].choice1 == 2 && games[index].choice2 == 3)
    {
        //sock 2 vicktory
        winner = 2;
    }
    else if(games[index].choice1 == 3 && games[index].choice2 == 1)
    {
        // sock 2 victory
        winner = 2;
    }
    else if(games[index].choice1 == 3 && games[index].choice2 == 2)
    {
        //sock 1 vicktory
        winner = 1;
    }
    else if(games[index].choice1 == 4 && games[index].choice2 != 4)
    {
        //sock 2 vicktory
        winner = 2;
    }
    else if(games[index].choice1 != 4 && games[index].choice2 == 4)
    {
        //sock 1 vicktory
        winner = 1;
    }
    
    return winner;
}

int main(int argc, char *argv[])
{

    if(argc!=2)
    {
        printf("To few arguments!\n");
        exit(1);
    }

    fill();
    initializeScore();
    char delim[]=":";
    char *Desthost=strtok(argv[1],delim);
    char *Destport=strtok(NULL,delim);


    struct sockaddr_storage clientAddr;
    socklen_t addrLenght;

    struct addrinfo hints, *p, *servinfo;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000000;
    
    std::string tempString;
    std::string bufString;
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
    char roundF[50];
    char win[50];
    char lose[50];
    char choose[50];
    char operation[256];
    char finished[50];
    char charHighScore[4000];
    char bigBuf2[4011];

    memset(buf,0,sizeof(buf));
    
    int yes = 1;
    int recivedValue;
    int sendValue;
    int choice = 0;
    int answer = 0;
    int winner = 0;
    int gameToWatch = 0;
    int savedScores = 0;
    double tempDouble = 0.0f; 
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

                            nrOfPlayers++;
                            if((nrOfPlayers % 2) == 0)
                            {
                                //Start game between the two index nummbers found in Index and then gameCounter++
                                games[gameCounter].sockNr2 = i;
                                printf("Game Ready to start\n");
                                                                                                     
                                sendValue = send(games[gameCounter].sockNr1,gameStartingMsg,strlen(gameStartingMsg),0);
                                if(sendValue < 0)
                                {
                                    printf("Error sending hello msg\n");
                                    //close(acceptFd);
                                    break;
                                }                                    

                                sendValue = send(games[gameCounter].sockNr2,gameStartingMsg,strlen(gameStartingMsg),0);
                                if(sendValue < 0)
                                {
                                    printf("Error sending hello msg\n");
                                    //close(acceptFd);
                                    break;
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
                            int tempCounter = 0;
                            printf("Watch\n");
                            for(int j = 0; j < gameCounter; j++)
                            {
                                if(games[j].started == true)
                                {
                                    tempCounter++;
                                }                                
                            }
                            printf("Tempcounter = %d\n",tempCounter);
                            memset(choose,0,sizeof(choose));
                            sprintf(choose,"CHOOSE %d",tempCounter);
                            sendValue = send(i,choose,strlen(choose),0);
                            if(sendValue < 0)
                            {
                                printf("Error sending hello msg\n");
                                //close(acceptFd);
                                break;
                            }
                            fflush(stdout);
                                                 
                        }
                        else if(choice == 3)
                        {
                            scoreList[0].looseScore = 1;
                            scoreList[0].winnerScore = 3;
                            scoreList[1].looseScore = 2;
                            scoreList[1].winnerScore = 3;
                            savedScores = 2;
                            memset(bigBuf2,0,sizeof(bigBuf2));
                            memset(charHighScore,0,sizeof(charHighScore));
                            printf("HIGSCORE LIST:");
                            for(int i = 0; i < savedScores; i++)
                            {
                                bufString = "Score: " + std::to_string(scoreList[i].winnerScore) + " - " + std::to_string(scoreList[i].looseScore) + "\n";
                                tempString += bufString;
                            }
                            std::cout << tempString << std::endl;
                            strcpy(charHighScore,tempString.c_str());
                            tempString = "";
                            sprintf(bigBuf2,"Highscore \n%s",charHighScore);

                            sendValue = send(i,bigBuf2,strlen(bigBuf2),0);
                            if(sendValue < 0)
                            {
                                printf("Error sending hello msg\n");
                                //close(acceptFd);
                                break;
                            }
                            printf("send MSG = %s size%d\n",bigBuf2, sendValue);

                            fflush(stdout);
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
                    else if(strcmp(operation, "STOPW") == 0)
                    {
                        //games[i].watching[games[i].nrOfWatching]
                        
                        for(int j = 0; j < gameCounter; j++)
                        {
                            for(int k = 0; k < games[j].nrOfWatching; k++)
                            {
                                if(games[j].watching[k] == i)
                                {
                                    games[j].watching[k] = games[j].watching[games[j].nrOfWatching];
                                    games[j].nrOfWatching--;
                                }
                            }
                        }

                        //see how many client games that is active.
                        int tempCounter = 0;                        
                        for(int j = 0; j < gameCounter; j++)
                        {
                            if(games[j].started == true)
                            {
                                tempCounter++;
                            }                            
                        }
                        memset(choose,0,sizeof(choose));
                        sprintf(choose,"CHOOSE %d",tempCounter);
                        sendValue = send(i,choose,strlen(choose),0);
                        if(sendValue < 0)
                        {
                            printf("Error sending hello msg\n");
                            //close(acceptFd);
                            break;
                        }
                        fflush(stdout);

                    }
                    else if(strcmp(operation, "STOPC") == 0)
                    {
                        //STOP CHOOSING
                        
                        sendValue = send(i,menuMsg,strlen(menuMsg),0);
                        if(sendValue < 0)
                        {
                            printf("Error sending hello msg\n");
                            //close(acceptFd);
                            break;
                        }
                    }
                    else if(strcmp(operation, "WATCH") == 0)
                    {
                        //lägg till så att client börjar titta
                        sscanf(buf,"%s %d",operation, &gameToWatch);
                        // search for the client to watch:
                        int tempCounter = 0;
                        for(int j = 0; j < gameCounter; j++)
                        {
                            if(games[j].started == true)
                            {
                                tempCounter++;
                            }
                            if(tempCounter == gameToWatch)
                            {
                                //added client to watching list.
                                games[j].watching[games[j].nrOfWatching] = i;
                                games[j].nrOfWatching++;
                                break;
                            }                        
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
                             
                            }
                            if(games[j].sockNr1 == i)
                            {
                                games[j].socket1Ready = true;
                             
                            }                                                       
                        }
                        for(int j = 0; j < gameCounter; j++)
                        {
                            if(games[j].socket1Ready == true && games[j].socket2Ready == true && games[j].started != true)
                            {
                                // Starta spelet
                                printf("Båda klienter redo!\n");  
                                games[j].started = true;                                                            
                                sendTimingMsg(j);
                                
                                fflush(stdout);      
                            }
                        }
                       
                    }
                    else if(strcmp(operation, "ROUND") == 0)
                    {
                        sscanf(buf,"%s %d %lf",operation, &answer, &tempDouble);
                        for(int j = 0; j < gameCounter; j++)
                        {
                            printf("for loop test\n");
                            if(games[j].sockNr1 == i)
                            {
                                printf("socket1 answer saved\n");
                                games[j].choice1 = answer;
                                printf("Time taken %8.8g\n",tempDouble);
                                games[j].timeTaken1 += tempDouble;
                                tempDouble = 0.0f;                                
                                answer = 0;
                            }
                            if(games[j].sockNr2 == i)
                            {
                                printf("socket2 answer saved\n");
                                games[j].choice2 = answer;
                                printf("Time taken %8.8g\n",tempDouble);
                                games[j].timeTaken2 += tempDouble;
                                tempDouble = 0.0f;
                                answer = 0;
                            }
                            if(games[j].choice1 != 0 && games[j].choice2 != 0)
                            {
                                printf("Both answerd\n");
                                winner = rockPapperScissors(j);

                                if(winner == 1)
                                {
                                    
                                    //sock 1 winner
                                                                        
                                    games[j].score1++;
                                    if(games[j].score1 == 3)
                                    {
                                        games[j].secondsToCount = -1;
                                        games[j].concluded = true;
                                                                       
                                        memset(win,0,sizeof(win));
                                        sprintf(win,"WIN %d %d\n",games[j].score1,games[j].score2);                            
                                        sendValue = send(games[j].sockNr1,win,strlen(win),0);
                                        if(sendValue < 0)
                                        {
                                            printf("Error sending hello msg\n");                                    
                                            exit(4);
                                        }
                                        fflush(stdout);
                                        memset(lose,0,sizeof(lose));                                   
                                        sprintf(lose,"LOSE %d %d\n",games[j].score2,games[j].score1);
                                        
                                        sendValue = send(games[j].sockNr2,lose,strlen(lose),0);
                                        if(sendValue < 0)
                                        {
                                            printf("Error sending hello msg\n");                                    
                                            exit(4);
                                        }
                                        memset(finished,0,sizeof(finished));
                                        sprintf(finished,"FINISHED %d %d\n", games[j].score1, games[j].score2); 
                                        for(int k = 0; k < games[j].nrOfWatching; k++)
                                        {                                                                                                                   
                                            sendValue = send(games[j].watching[k],finished,strlen(finished),0);
                                            if(sendValue < 0)
                                            {
                                                printf("Error sending hello msg\n");                                    
                                                exit(4);
                                            }
                                            printf("sendbytes1: %d\n", sendValue);
                                            fflush(stdout);
                                        }
                                        
                                        games[j].sockNr1 = -1;
                                        games[j].sockNr2 = -1; 
                                        //SaveHighScore
                                        scoreList[savedScores].looseScore = games[j].score2;
                                        scoreList[savedScores++].winnerScore = games[j].score1;                                        
                                        fflush(stdout);
                                    }
                                    else
                                    {
                                        printf("sock 1 won\n");                                    
                                        memset(roundF,0,sizeof(roundF));
                                        sprintf(roundF,"ROUNDF %d %d\n",games[j].score1,games[j].score2);                            
                                        sendValue = send(games[j].sockNr1,roundF,strlen(roundF),0);
                                        if(sendValue < 0)
                                        {
                                            printf("Error sending hello msg\n");                                    
                                            exit(4);
                                        }
                                        fflush(stdout);
                                        memset(roundF,0,sizeof(roundF));                                   
                                        sprintf(roundF,"ROUNDF %d %d\n",games[j].score2,games[j].score1);
                                        
                                        sendValue = send(games[j].sockNr2,roundF,strlen(roundF),0);
                                        if(sendValue < 0)
                                        {
                                            printf("Error sending hello msg\n");                                    
                                            exit(4);
                                        }
                                        fflush(stdout);
                                        memset(roundF,0,sizeof(roundF));
                                        sprintf(roundF,"ROUNDF %d %d\n",games[j].score1,games[j].score2);
                                        for(int k = 0; k < games[j].nrOfWatching; k++)
                                        {                                                                                                                   
                                            sendValue = send(games[j].watching[k],roundF,strlen(roundF),0);
                                            if(sendValue < 0)
                                            {
                                                printf("Error sending hello msg\n");                                    
                                                exit(4);
                                            }
                                            printf("sendbytes1: %d\n", sendValue);
                                            fflush(stdout);
                                        }


                                        games[j].choice1 = 0;
                                        games[j].choice2 = 0;
                                        
                                        games[j].secondsToCount = 3;
                                        games[j].isAnswering = false;
                                        fflush(stdout);
                                    }
                                }
                                else if(winner == 2)
                                {
                                    //sock 2 winner
                                    games[j].score2++;
                                    
                                    if(games[j].score2 == 3)
                                    {
                                        games[j].secondsToCount = -1;
                                        games[j].concluded = true;
                                        
                                        memset(lose,0,sizeof(lose));
                                        sprintf(lose,"LOSE %d %d\n",games[j].score1,games[j].score2);                            
                                        sendValue = send(games[j].sockNr1,lose,strlen(lose),0);
                                        if(sendValue < 0)
                                        {
                                            printf("Error sending hello msg\n");                                    
                                            exit(4);
                                        }
                                        fflush(stdout);
                                        memset(win,0,sizeof(win));                                 
                                        sprintf(win,"WIN %d %d\n",games[j].score2,games[j].score1);
                                        
                                        sendValue = send(games[j].sockNr2,win,strlen(win),0);
                                        if(sendValue < 0)
                                        {
                                            printf("Error sending hello msg\n");                                    
                                            exit(4);
                                        }  

                                        memset(finished,0,sizeof(finished));
                                        sprintf(finished,"FINISHED %d %d\n", games[j].score1, games[j].score2); 
                                        for(int k = 0; k < games[j].nrOfWatching; k++)
                                        {                                                                                                                   
                                            sendValue = send(games[j].watching[k],finished,strlen(finished),0);
                                            if(sendValue < 0)
                                            {
                                                printf("Error sending hello msg\n");                                    
                                                exit(4);
                                            }
                                            printf("sendbytes1: %d\n", sendValue);
                                            fflush(stdout);
                                        }

                                        games[j].sockNr1 = -1;
                                        games[j].sockNr2 = -1;
                                        // Save highscore                                        
                                        scoreList[savedScores].looseScore = games[j].score1;
                                        scoreList[savedScores++].winnerScore = games[j].score2;
                                        fflush(stdout);

                                    }
                                    else
                                    {
                                        
                                        printf("sock 2 won\n");
                                        
                                        memset(roundF,0,sizeof(roundF));
                                        sprintf(roundF,"ROUNDF %d %d\n",games[j].score1,games[j].score2);                            
                                        sendValue = send(games[j].sockNr1,roundF,strlen(roundF),0);
                                        if(sendValue < 0)
                                        {
                                            printf("Error sending hello msg\n");                                    
                                            exit(4);
                                        }
                                        fflush(stdout);
                                        memset(roundF,0,sizeof(roundF));                                   
                                        sprintf(roundF,"ROUNDF %d %d\n",games[j].score2,games[j].score1);
                                        
                                        sendValue = send(games[j].sockNr2,roundF,strlen(roundF),0);
                                        if(sendValue < 0)
                                        {
                                            printf("Error sending hello msg\n");                                    
                                            exit(4);
                                        }
                                        fflush(stdout);

                                        memset(roundF,0,sizeof(roundF));
                                        sprintf(roundF,"ROUNDF %d %d\n",games[j].score1,games[j].score2);

                                        for(int k = 0; k < games[j].nrOfWatching; k++)
                                        {                                                                                                                   
                                            sendValue = send(games[j].watching[k],roundF,strlen(roundF),0);
                                            if(sendValue < 0)
                                            {
                                                printf("Error sending hello msg\n");                                    
                                                exit(4);
                                            }
                                            printf("sendbytes1: %d\n", sendValue);
                                            fflush(stdout);
                                        }

                                        games[j].choice1 = 0;
                                        games[j].choice2 = 0;
                                        
                                        games[j].secondsToCount = 3;
                                        games[j].isAnswering = false;

                                    }                                 
                                    
                                }
                                else
                                {
                                    //draw
                                    
                                    memset(roundF,0,sizeof(roundF));
                                    sprintf(roundF,"ROUNDF %d %d\n",games[j].score1,games[j].score2);                            
                                    sendValue = send(games[j].sockNr1,roundF,strlen(roundF),0);
                                    if(sendValue < 0)
                                    {
                                        printf("Error sending hello msg\n");                                    
                                        exit(4);
                                    }
                                    fflush(stdout);
                                    memset(roundF,0,sizeof(roundF));
                                    sprintf(roundF,"ROUNDF %d %d\n",games[j].score2,games[j].score1);
                                    sendValue = send(games[j].sockNr2,roundF,strlen(roundF),0);
                                    if(sendValue < 0)
                                    {
                                        printf("Error sending hello msg\n");                                    
                                        exit(4);
                                    }

                                    memset(roundF,0,sizeof(roundF));
                                    sprintf(roundF,"ROUNDF %d %d\n",games[j].score1,games[j].score2);

                                    for(int k = 0; k < games[j].nrOfWatching; k++)
                                    {                                                                                                                   
                                        sendValue = send(games[j].watching[k],roundF,strlen(roundF),0);
                                        if(sendValue < 0)
                                        {
                                            printf("Error sending hello msg\n");                                    
                                            exit(4);
                                        }
                                        printf("sendbytes1: %d\n", sendValue);
                                        fflush(stdout);
                                    }

                                    games[j].choice1 = 0;
                                    games[j].choice2 = 0;
                                    
                                    games[j].secondsToCount = 3;
                                    games[j].isAnswering = false;
                                    fflush(stdout);
                                }
                                
                                if(games[j].sockNr1 != 3 || games[j].score2 != 3)
                                {
                                    printf("HERE\n");
                                    sendTimingMsg(j);
                                }                                
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
                    if((comparetime.tv_sec - games[i].time.tv_sec) > 1 && games[i].started == true && games[i].isAnswering == false && games[i].concluded == false)
                    {                
                        if(games[i].score1 != 3 || games[i].score2 != 3)
                        {
                            
                            sendTimingMsg(i);
                        }                        
                    }
                }
            }
        }      
    }


    return 0;
}
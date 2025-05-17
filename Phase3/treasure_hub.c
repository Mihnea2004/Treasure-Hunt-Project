#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define CMD_FILE "command.txt"
pid_t monitorPid = -1;
int monitorRunning = 0;
int monitorStopping = 0;

typedef struct {
    int id;
    char username[32];
    float latitude;
    float longitude;
    char clueText[128];
    int value;
} Treasure;

int findNumberOfTreasures(const char *hunt_id){
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/treasures.dat", hunt_id);
    
    struct stat st;
    if(stat(hunt_id, &st) != 0){
        perror("stat");
        exit(-1);
    } 
    else if(S_ISDIR(st.st_mode) == 0){
        perror("Hunt_id is not a directory!\n");
        exit(-1);
    }

    int fd = open(filePath, O_RDONLY);
    if(fd == -1){
        perror("An error occured while trying to open the file!\n");
        exit(-1);
    }

    Treasure t;
    ssize_t r;
    int counter = 0;
    while((r = read(fd, &t, sizeof(Treasure))) == sizeof(Treasure))
        counter++;

    if(r != 0){
        perror("An error occured while trying to read the treasures!\n");
        exit(-1);
    }

    if(close(fd) < 0){
        perror("An error occured while trying to close the file!\n");
        exit(-1);
    }

    return counter;
}

void listHunts(){
    DIR *dir = opendir(".");
    if(dir == NULL){
        perror("An error occured while trying to open the directory!\n");
        exit(-1);
    }
    
    struct dirent *entry;
    int found = 0;
    
    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        if(entry->d_type == DT_DIR){
            found = 1;
            printf("Hunt Name: %s, Number of treasures: %d\n", entry->d_name, findNumberOfTreasures(entry->d_name));
        }
    }

    if(found == 0)
        printf("There are no hunts found!\n");

    if(closedir(dir) < 0){
        perror("An error occured while trying to close the directory!\n");
        exit(-1);
    }
}

void printTreasure(Treasure *t){
    char s[512];
    snprintf(s, sizeof(s), "ID: %d, Username: %s, Latitude: %.2f, Longitude: %.2f, ClueText: %s, Value: %d;\n", t->id, t->username, t->latitude, t->longitude, t->clueText, t->value);
    write(1, s, strlen(s));
}

void writeString(char *s) {
    write(STDOUT_FILENO, s, strlen(s));
}

void listTreasures(const char *hunt_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/treasures.dat", hunt_id);

    struct stat st;
    if(stat(path, &st) < 0){
        perror(NULL);
        exit(-1);
    }

    char info[256];
    snprintf(info, sizeof(info), "Hunt: %s\nFile size: %ld\nLast modified: %ld\n", hunt_id, st.st_size, st.st_mtime);
    writeString(info);

    int fd = open(path, O_RDONLY);
    if(fd < 0){
        perror(NULL);
        exit(-1);
    }

    Treasure t;
    while(read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        char line[256];
        snprintf(line, sizeof(line), "- ID: %d | User: %s | Value: %d\n", t.id, t.username, t.value);
        writeString(line);
    }

    if(close(fd) < 0){
        perror("An error occurred while trying to close the file!\n");
        exit(-1);
    }
}

void viewTreasures(const char *hunt_id, int id){
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/treasures.dat", hunt_id);

    struct stat st;
    if(stat(hunt_id, &st) != 0){
        perror("Path to this directory doesn't exist!\n");
        exit(-1);
    }
    else if(S_ISDIR(st.st_mode) == 0){
        perror("Not a directory!\n");
        exit(-1);
    }

    int fd = open(filePath, O_RDONLY);
    if(fd < 0){
        perror("treasure.dat file cannot be open!\n");
        exit(-1);
    }

    Treasure t;
    ssize_t r;
    int found = 0;

    while((r = read(fd, &t, sizeof(Treasure))) == sizeof(Treasure)){
        if(t.id == id){
            found = 1;
            break;
        }
    }

    if(found)
        printTreasure(&t);
    else{
        write(1, "Not found!\n", 11);
    }

    if(close(fd) < 0){
        perror("An error occurred while trying to close the file!\n");
        exit(-1);
    }
}

void calculateScore(){
   DIR *dir = opendir(".");
    if(dir == NULL){
        perror("An error occured while trying to open the directory!\n");
        exit(-1);
    }
    
    struct dirent *entry;

    int pipeMonitorNephews[2];
    if(pipe(pipeMonitorNephews) < 0){
        perror("An error occured while trying to create the pipe!\n");
        exit(-1);
    }

    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        if(entry->d_type == DT_DIR){
            pid_t pid = fork();
            if(pid < 0){
                perror("An error occured while trying to create the proccess!\n");
                exit(-1);
            }
            else if(pid == 0){
                close(pipeMonitorNephews[0]); //doesn't read anymore!
                dup2(pipeMonitorNephews[1], 1); //forwards stdout to pipeMonitorNephews
                close(pipeMonitorNephews[1]);

                execlp("./scoreCalculator", "./scoreCalculator", entry->d_name, NULL);
                perror("An error occured at execlp!\n");
                exit(-1);
            }
        }
    }

    close(pipeMonitorNephews[1]);
    
    while(wait(NULL) > 0); //waits for all nephews to end

    char buffer[512];
    ssize_t bytes;
    while((bytes = read(pipeMonitorNephews[0], buffer, sizeof(buffer) - 1))){
        buffer[bytes] = '\0';
        printf("%s", buffer);
    }
    close(pipeMonitorNephews[0]);
    if(closedir(dir) == -1){
        perror("An error occured while trying to close the directory!\n");
        exit(-1);
    }
}

void handler1(int sig){
    char buffer[256];
    int fd = open(CMD_FILE, O_RDONLY);
    if(fd < 0){
        perror("An error occurred while trying to open command file!\n");
        exit(-1);
    }

    int r = read(fd, buffer, sizeof(buffer) - 1);
    if(r <= 0){
        perror("An error occurred while trying to read from the file!\n");
        close(fd);
        exit(-1);
    }

    buffer[r] = '\0';

    if(close(fd) < 0){
        perror("An error occurred while trying to close the file!\n");
        exit(-1);
    }

    char *token = strtok(buffer," \n");
    if(token == NULL)
        return;

    if(strcmp(token, "listHunts") == 0)
        listHunts();
    else if(strcmp(token, "listTreasures") == 0){
        char *hunt_id = strtok(NULL," \n");
        if(hunt_id) 
            listTreasures(hunt_id);
        else 
            printf("Hunt ID is missing!\n");
    }
    else if(strcmp(token, "viewTreasure") == 0){
        char *hunt_id = strtok(NULL," \n");
        char *id = strtok(NULL," \n");
        if(hunt_id && id) 
            viewTreasures(hunt_id, strtol(id, NULL, 10));
        else 
            printf("Hunt ID or ID is missing!\n");
    }
    else if(strcmp(token, "calculateScore") == 0){
        calculateScore();
    }
    else
        printf("Error! Try another command!\n");
}

void handler2(int sig){
    printf("Stopping monitor!\n");
    usleep(3000000);
    exit(0);
}

void handlerSigchld(int sig){
    int status;
    waitpid(monitorPid, &status, 0);

    printf("\nMonitor ended with code: %d\n", WEXITSTATUS(status));

    monitorRunning = 0;
    monitorPid = -1;
    monitorStopping = 0;
}

void monitorLoop(){
    struct sigaction s1, s2;
    s1.sa_handler = handler1;
    sigemptyset(&s1.sa_mask);
    s1.sa_flags = 0;
    sigaction(SIGUSR1, &s1, NULL);

    s2.sa_handler = handler2;
    sigemptyset(&s2.sa_mask);
    s2.sa_flags = 0;
    sigaction(SIGUSR2, &s2, NULL);

    struct sigaction act_chld;
    act_chld.sa_handler = SIG_IGN; // ignores the SIGCHLD from nephews
    sigemptyset(&act_chld.sa_mask);
    act_chld.sa_flags = 0;
    sigaction(SIGCHLD, &act_chld, NULL);

    while(1)
        pause(); //Waiting for signals
}

void startMonitor(){
    if(monitorRunning){
        printf("Monitor is already running!\n");
        return;
    }

    pid_t pid = fork();
    if(pid < 0){
        perror("An error occurred while trying to create a child!\n");
        exit(-1);
    }
    else if(pid == 0){
        monitorLoop();
        exit(0);
    }
    else{
        monitorPid = pid;
        monitorRunning = 1;
        monitorStopping = 0;
        printf("Monitor has started with pid: %d\n", monitorPid);
    }
}

void writeCommand(const char *line){
    int fd = open(CMD_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if(fd < 0){
        perror("An error occurred while trying to open the file for writing!\n");
        exit(-1);
    }

    write(fd, line, strlen(line));

    kill(monitorPid, SIGUSR1);
    usleep(50000); //a small delay to ensure the signal is processed

    if(close(fd) < 0){
        perror("An error occurred while trying to close the file for writing!\n");
        exit(-1);
    }
}

int main(){
    struct sigaction sa;
    sa.sa_handler = handlerSigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);
    
    char s[256];
    while(1){
        printf("\033[35mprompt > \033[0m"); //print text in purple, then come back to the normal color
        fflush(stdout);

        if(fgets(s, sizeof(s), stdin) == NULL){
            clearerr(stdin); //avoiding an infinite loop of empty prompts
            continue;
        }

        s[strcspn(s, "\n")] = '\0';

        if(strcmp(s, "startMonitor") == 0)
            startMonitor();
        else if(strcmp(s, "listHunts") == 0){
            if(monitorRunning == 0) 
                printf("Monitor isn't running!\n");
            else if(monitorStopping == 1) 
                printf("Monitor is stopping, so you can't enter a new command!\n");
            else 
                writeCommand("listHunts");
        }
        else if(strncmp(s, "listTreasures", 13) == 0){
            if(monitorRunning == 0) 
                printf("Monitor isn't running!\n");
            else if(monitorStopping == 1) 
                printf("Monitor is stopping, so you can't enter a new command!\n");
            else 
                writeCommand(s);
        }
        else if(strncmp(s, "viewTreasure", 12) == 0){
            if(monitorRunning == 0) 
                printf("Monitor isn't running!\n");
            else if(monitorStopping == 1) 
                printf("Monitor is stopping, so you can't enter a new command!\n");
            else 
                writeCommand(s);
        }
        else if(strncmp(s, "calculateScore", 14) == 0){
            if(monitorRunning == 0) 
                printf("Monitor isn't running!\n");
            else if(monitorStopping == 1) 
                printf("Monitor is stopping, so you can't enter a new command!\n");
            else 
                writeCommand("calculateScore");
        }
        else if(strcmp(s, "stopMonitor") == 0){
            if(monitorRunning == 0) 
                printf("Monitor isn't running!\n");
            else{
                kill(monitorPid, SIGUSR2);
                usleep(1000);
                monitorStopping = 1;
            }
        }
        else if(strcmp(s, "exit") == 0){
            if(monitorRunning) 
                printf("Monitor is still running!\n");
            else 
                break;
        }
        else
            printf("Error! Try another command!\n");
    }

    return 0;
}
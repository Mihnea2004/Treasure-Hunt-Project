#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#define CMD_FILE "monitor_data/command.txt"
pid_t monitor_pid = -1;
int monitor_running = 0;
int monitor_stopping = 0;

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
    //check directory existence
    if(stat(hunt_id,&st)!=0){
        perror("stat");
        exit(-1);
    } 
    else if(S_ISDIR(st.st_mode) == 0){
        perror("not a directory");
        exit(-1);
    }

    //open file treasure.dat
    int fd = open(filePath, O_RDONLY);
    if(fd == -1){
        perror("open");
        exit(-1);
    }

    //read from treasures.dat file
    Treasure t;
    ssize_t r;
    int counter = 0;
    while((r = read(fd, &t, sizeof(Treasure))) == sizeof(Treasure))
        counter++;

    if(r != 0){
        perror("read");
        exit(-1);
    }

    if(close(fd) < 0){
        perror("close");
        exit(-1);
    }

    return counter;
}

void listHunts(){
    DIR *dir = opendir(".");
    if(dir == NULL){
        perror("opendir");
        exit(-1);
    }
    
    struct dirent *st;
    int found = 0;
    
    while((st = readdir(dir)) != NULL){
        if(strcmp(st->d_name,".") == 0 || strcmp(st->d_name,"..") == 0)
            continue;
        
        if(st->d_type == DT_DIR){
            found = 1;
            printf("Hunt_Name: %s, Number of treasures: %d\n", st->d_name, findNumberOfTreasures(st->d_name));
        }
        
    }
    if(found == 0)
        printf("There are no hunts found!\n");

    closedir(dir);
}

void printTreasure(Treasure *t){
    char s[512];
    snprintf(s, sizeof(s), "ID: %d, Username: %s, Latitude: %.2f, Longitude: %.2f, ClueText: %s, Value: %d;\n", t->id, t->username, t->latitude, t->longitude, t->clueText, t->value);
    write(1, s, strlen(s));
}

void write_command(const char *cmd){
    int fd = open(CMD_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd < 0){
        perror("open");
        return;
    }
    write(fd, cmd, strlen(cmd));
    close(fd);
}

void handle_sigchld(int sig){
    int status;
    waitpid(monitor_pid, &status, 0);
    printf("[hub] Monitor terminated.\n");
}

int main(){
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    mkdir("monitor_data", 0755);
    write_command("TEST_COMMAND");

    pid_t pid = fork();
    if(pid == 0)
        _exit(0);
    else if(pid > 0){
        monitor_pid = pid;
        int status;
        wait(&status);
    }
    else
        perror("fork");

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

typedef struct {
    int id;
    char username[32];
    float latitude;
    float longitude;
    char clueText[128];
    int value;
} Treasure;

int readInput(int fd, char *buffer, int size) {
    int index = 0;
    char temp[1024];

    while(index < size - 1){
        int r = read(fd, temp, sizeof(temp));
        if(r <= 0)
            break;

        for(int i = 0; i < r && index < size - 1; i++){
            if(temp[i] == '\n') {  
                buffer[index] = '\0';
                return index;
            }
            buffer[index++] = temp[i];
        }
    }
    buffer[index] = '\0';  
    return index;
}

void writeString(char *s) {
    write(STDOUT_FILENO, s, strlen(s));
}

void logOperation(const char *hunt_id, char *message) {
    char log_path[256];
    char symlink_name[256];

    snprintf(log_path, sizeof(log_path), "%s/logged_hunt", hunt_id);
    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd >= 0) {
        write(fd, message, strlen(message));
        write(fd, "\n", 1);
        close(fd);
    }

    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name);
    symlink(log_path, symlink_name);
}

void addTreasure(char *hunt_id) {
    mkdir(hunt_id, 0755);

    char path[256];
    snprintf(path, sizeof(path), "%s/treasures.dat", hunt_id);
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd < 0) 
        return;

    Treasure t;
    char buffer[128];

    writeString("Treasure ID: ");
    readInput(STDIN_FILENO, buffer, sizeof(buffer));
    t.id = atoi(buffer);

    writeString("Username: ");
    readInput(STDIN_FILENO, t.username, sizeof(t.username));

    writeString("Latitude: ");
    readInput(STDIN_FILENO, buffer, sizeof(buffer));
    t.latitude = strtof(buffer, NULL);

    writeString("Longitude: ");
    readInput(STDIN_FILENO, buffer, sizeof(buffer));
    t.longitude = strtof(buffer, NULL);

    writeString("Clue: ");
    readInput(STDIN_FILENO, t.clueText, sizeof(t.clueText));

    writeString("Value: ");
    readInput(STDIN_FILENO, buffer, sizeof(buffer));
    t.value = atoi(buffer);

    write(fd, &t, sizeof(Treasure));
    close(fd);

    logOperation(hunt_id, "Add treasure executed\n");
}

void listTreasures(const char *hunt_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/treasures.dat", hunt_id);

    struct stat st;
    if(stat(path, &st) < 0) 
        return;

    char info[256];
    snprintf(info, sizeof(info), "Hunt: %s\nFile size: %ld\nLast modified: %ld\n", hunt_id, st.st_size, st.st_mtime);
    writeString(info);

    int fd = open(path, O_RDONLY);
    if(fd < 0) 
        return;

    Treasure t;
    while(read(fd, &t, sizeof(t)) == sizeof(t)) {
        char line[256];
        snprintf(line, sizeof(line), "- ID: %d | User: %s | Value: %d\n", t.id, t.username, t.value);
        writeString(line);
    }

    close(fd);
    logOperation(hunt_id, "List treasures executed");
}

int main(int argc, char **argv){
    if(argc < 3){
        perror(NULL);
        return 1;
    }

    if(strcmp(argv[1], "add") == 0)
        addTreasure(argv[2]);
    else if(strcmp(argv[1], "list") == 0)
        listTreasures(argv[2]);
    else{
        writeString("Error! Try another command\n");
        return 1;
    }

    return 0;
}

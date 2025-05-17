#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
    
typedef struct {
    int id;
    char username[32];
    float latitude;
    float longitude;
    char clueText[128];
    int value;
} Treasure;

ssize_t readInput(int fd, char *buffer, size_t size) {
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

//log operations related to a treasure hunt
void logOperation(const char *hunt_id, char *message) {
    char log_path[256];
    char symlink_name[256];

    snprintf(log_path, sizeof(log_path), "%s/logged_hunt", hunt_id);
    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd >= 0) {
        write(fd, message, strlen(message)); //write in log file
        write(fd, "\n", 1);
        close(fd);
    }

    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt - %s", hunt_id); //symlink name
    unlink(symlink_name); //remove symlink with the same name
    symlink(log_path, symlink_name); //create the symlink to the log file named 'symlink_name'
}

void addTreasure(char *hunt_id) {
    if(access(hunt_id, F_OK) == -1){
        if(mkdir(hunt_id, 0755) < 0){
            perror("An error occurred while trying to create the directory!\n");
            exit(-1);
        }
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/treasures.dat", hunt_id);
    int fd = open(path, O_RDWR | O_CREAT, 0644);
    if(fd < 0){
        perror(NULL);
        exit(-1);
    }

    char buffer[128];
    Treasure t;

    while(1){
        //ID
        while(1){
            writeString("Treasure ID: ");
            readInput(STDIN_FILENO, buffer, sizeof(buffer));
            int valid = 1;
            for(int i = 0; buffer[i] && buffer[i] != '\n'; i++) {
                if(buffer[i] < '0' || buffer[i] > '9') {
                    valid = 0;
                    break;
                }
            }
            if(!valid){
                write(1, "Invalid input, you must enter an integer number!\n", 50);
                continue;
            }

            t.id = atoi(buffer);

            // verificare duplicat ID
            lseek(fd, 0, SEEK_SET);
            Treasure temp;
            int duplicate = 0;
            while(read(fd, &temp, sizeof(Treasure)) == sizeof(Treasure)) {
                if(temp.id == t.id) {
                    duplicate = 1;
                    break;
                }
            }

            if(duplicate){
                write(1, "A treasure with this ID already exists in the hunt!\n", 53);
                continue;
            }

            break;
        }

        //username
        while(1){
            writeString("Username: ");
            readInput(STDIN_FILENO, t.username, sizeof(t.username));
            int onlyDigits = 1;
            for(int i = 0; t.username[i] && t.username[i] != '\n'; i++) {
                if(t.username[i] < '0' || t.username[i] > '9') {
                    onlyDigits = 0;
                    break;
                }
            }
            if(!onlyDigits) 
                break;
            else 
                write(1, "Invalid input, you must enter a string!\n", 41);
        }

        //latitude
        while(1){
            writeString("Latitude: ");
            readInput(STDIN_FILENO, buffer, sizeof(buffer));
            char *endptr;
            t.latitude = strtof(buffer, &endptr);
            if(endptr != buffer && *endptr == '\0') break;
            else write(1, "Invalid input, you must enter a floating point number!\n", 56);
        }

        //longitude
        while(1){
            writeString("Longitude: ");
            readInput(STDIN_FILENO, buffer, sizeof(buffer));
            char *endptr;
            t.longitude = strtof(buffer, &endptr);
            if(endptr != buffer && *endptr == '\0') 
                break;
            else 
                write(1, "Invalid input, you must enter a floating point number!\n", 56);
        }

        //clue
        while(1){
            writeString("Clue: ");
            readInput(STDIN_FILENO, t.clueText, sizeof(t.clueText));
            int onlyDigits = 1;
            for(int i = 0; t.clueText[i] && t.clueText[i] != '\n'; i++) {
                if(t.clueText[i] < '0' || t.clueText[i] > '9') {
                    onlyDigits = 0;
                    break;
                }
            }
            if(!onlyDigits) 
                break;
            else 
                write(1, "Invalid input, you must enter a string!\n", 41);
        }

        //value
        while(1){
            writeString("Value: ");
            readInput(STDIN_FILENO, buffer, sizeof(buffer));
            int valid = 1;
            for(int i = 0; buffer[i] && buffer[i] != '\n'; i++) {
                if(buffer[i] < '0' || buffer[i] > '9') {
                    valid = 0;
                    break;
                }
            }
            if(valid){
                t.value = atoi(buffer);
                break;
            }
            else 
                write(1, "Invalid input, you must enter an integer number!\n", 50);
        }

        //writing in treasure.dat file
        lseek(fd, 0, SEEK_END); //longseek to the end
        if(write(fd, &t, sizeof(Treasure)) != sizeof(Treasure)){
            perror("Failed to write treasure");
            close(fd);
            exit(-1);
        }

        char option[8];
        writeString("\nAdd another treasure? (yes/no): ");
        readInput(STDIN_FILENO, option, sizeof(option));
        if(strncmp(option, "no", 2) == 0)
            break;
    }

    close(fd);
    logOperation(hunt_id, "Adding treasures operation was successfully accomplished!\n");
}


void printTreasure(Treasure *t){
    char s[512];
    snprintf(s, sizeof(s), "ID: %d, Username: %s, Latitude: %.2f, Longitude: %.2f, ClueText: %s, Value: %d;\n", t -> id, t -> username, t -> latitude, t -> longitude, t -> clueText, t -> value);
    write(1, s, strlen(s));
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
        perror("An error occured while trying to close the file!\n");
        exit(-1);
    }

    logOperation(hunt_id, "List treasures operation was succesfully accomplished!\n");
}

void viewTreasures(const char *hunt_id, int id){
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/treasures.dat", hunt_id);

    struct stat st;

    //check directory existence
    if(stat(hunt_id, &st) != 0){
        perror("Path to this directory doesn't exist!");
        exit(-1);
    }
    else if(S_ISDIR(st.st_mode) == 0){
        perror("Not a directory!");
        exit(-1);
    }

    //open treasure.dat file
    int fd = open(filePath, O_RDONLY);
    if(fd < 0){
        perror("treasure.dat file cannot be open!");
        exit(-1);
    }

    //read from treasure.dat file
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
        if(close(fd) < 0){
            perror("An error occured while trying to close the file!\n");
            exit(-1);
        }
        return;
    }

    if(close(fd) < 0){
        perror("An error occured while trying to close the file!\n");
        exit(-1);
    }

    //updating log_hunt
    char logPath[256], logMessage[256];
    snprintf(logPath, sizeof(logPath), "%s/logged_hunt", hunt_id);
    snprintf(logMessage, sizeof(logMessage), "Treasure with ID: %d was viewed!\n", id);
    logOperation(logPath, logMessage);
}

void removeTreasure(const char *hunt_id, int id){
    char filePath[256], tempFile[256];
    snprintf(filePath, sizeof(filePath), "%s/treasures.dat", hunt_id);
    snprintf(tempFile, sizeof(tempFile), "%s/temporary.dat", hunt_id);

    struct stat st;
    if(stat(hunt_id, &st) != 0 || !S_ISDIR(st.st_mode)){
        perror("Invalid hunt directory!\n");
        exit(-1);
    }

    int fd = open(filePath, O_RDONLY);
    if(fd < 0){
        perror("treasure.dat file cannot be opened!\n");
        exit(-1);
    }

    int out = open(tempFile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if(out == -1){
        perror("An error occurred while trying to open temporary file!\n");
        close(fd);
        exit(-1);
    }

    Treasure t;
    int found = 0;
    ssize_t r;

    while((r = read(fd, &t, sizeof(Treasure))) == sizeof(Treasure)){
        if(t.id == id){
            found = 1;
            continue;
        }
        if(write(out, &t, sizeof(Treasure)) != sizeof(Treasure)){
            perror("Error writing to temporary file!");
            close(fd);
            close(out);
            exit(-1);
        }
    }

    if(r == -1){
        perror("Error while reading from the file!");
        close(fd);
        close(out);
        exit(-1);
    }

    if(close(fd) < 0){
        perror("An error occured while trying to close the file!\n");
        exit(-1);
    }

    if(close(out) < 0){
        perror("An error occured while trying to close the file!\n");
        exit(-1);
    }

    if(found){
        if(remove(filePath) != 0 || rename(tempFile, filePath) != 0){
            perror("Replacing files failed!");
            exit(-1);
        }

        char logPath[256], logMessage[256];
        snprintf(logPath, sizeof(logPath), "%s/logged_hunt", hunt_id);
        snprintf(logMessage, sizeof(logMessage), "Treasure with ID: %d was removed!\n", id);
        logOperation(logPath, logMessage);

        write(1, "Treasure deleting was successfully accomplished!\n", 50);
    }
    else{
        remove(tempFile);
        write(1, "Treasure couldn't be found!\n", 28);
    }
}


void removeHunt(const char *hunt_id){
    char filePath[256], logPath[256], linkPath[256];
    snprintf(filePath, sizeof(filePath), "%s/treasures.dat", hunt_id);
    snprintf(logPath, sizeof(logPath), "%s/logged_hunt", hunt_id);
    snprintf(linkPath, sizeof(linkPath), "logged_hunt - %s", hunt_id);

    //check files, directories existance (hunt)
    struct stat st;
    if(stat(hunt_id, &st) != 0){
        perror("Path to this directory doesn't exist!\n");
        exit(-1);
    }
    else if(S_ISDIR(st.st_mode) == 0){
        perror("Not a directory!\n");
        exit(-1);
    }

    if(stat(filePath, &st) == 0 && S_ISREG(st.st_mode) != 0){
        if(remove(filePath) != 0){
            perror("An error occured while trying to delete treasure.dat file!\n");
            exit(-1);
        }
    }

    if(lstat(linkPath, &st) == 0 && S_ISLNK(st.st_mode) != 0){
        if(remove(linkPath) != 0){
            perror("An error occured while trying to delete the link file!\n");
            exit(-1);
        }
    }

    if(stat(logPath, &st) == 0 && S_ISREG(st.st_mode) != 0){
        if(remove(logPath) != 0){
            perror("An error occured while trying to delete logged_hunt file!\n");
            exit(-1);
        }
    }

    if(remove(hunt_id) != 0){
        perror("An error occured while trying to delete hunt directory!\n");
        exit(-1);
    }

    write(1, "Hunt deleting was succesfully accomplished!\n", 44);
}

int main(int argc, char **argv){
    if(argc == 1 || argc > 4){
        perror("Invalid number of arguments!\n");
        return 1;
    }

    if(strcmp(argv[1], "add") == 0 && argc == 3)
        addTreasure(argv[2]);
    else if(strcmp(argv[1], "list") == 0 && argc == 3)
        listTreasures(argv[2]);
    else if(strcmp(argv[1], "remove_hunt") == 0 && argc == 3)
        removeHunt(argv[2]);
    else if(strcmp(argv[1], "view") == 0 && argc == 4)
        viewTreasures(argv[2], atoi(argv[3]));
    else if(strcmp(argv[1], "remove_treasure") == 0 && argc == 4)
        removeTreasure(argv[2], atoi(argv[3]));
    else{
        writeString("Error! Try another command!\n");
        return 1;
    }

    return 0;
}

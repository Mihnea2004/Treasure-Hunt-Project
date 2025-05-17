#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>

#define SIZE_MAX_USERS 100

typedef struct{
   int id;
   char name[32];
   float latitude;
   float longitude;
   char clue[128];
   int value;
} Treasure;

typedef struct{
   char name[32];
   int total;
} Score;

void initialiseScore(Score *scores, int index){
   for(int i = 0; i < index; i++){
      strcpy(scores[i].name, "");
      scores[i].total = 0;
   }
}

void findHuntScore(const char *hunt_id){
   char file_path[256];
   snprintf(file_path,sizeof(file_path), "%s/treasures.dat", hunt_id);

   struct stat st;
   //chack directory existence
   if(stat(hunt_id, &st) != 0){
      perror("This hunt doesn't exist!\n");
      return;
   }
   else if(S_ISDIR(st.st_mode) == 0){
      perror("It is not a directory!\n");
      return;
   }

   //open treasures.dat file
   int fd = open(file_path, O_RDONLY);
   if(fd == -1){
      perror("An error occured while trying to open the file!\n");
      exit(-1);
   }

   //read from file
   Score array[SIZE_MAX_USERS];
   initialiseScore(array, SIZE_MAX_USERS);
   int index = 0; //number of users initially
   Treasure t;
   ssize_t bytes;
   while((bytes = read(fd, &t, sizeof(Treasure))) == sizeof(Treasure)){
      int found = 0;
      for(int i = 0; i < index; i++){
         if(strcmp(t.name, array[i].name)==0){
            found = 1;
            array[i].total += t.value;
            break;
         }
      }
      if(found == 0){
         strcpy(array[index].name, t.name);
         array[index++].total = t.value;
      }
   }
   
   if(bytes != 0){
      printf("An error occured while trying to read from the file!\n");
      return;
   }

   if(close(fd) != 0){
      perror("An error occured while trying to close the file!\n");
      exit(-1);
   }
   
   printf("The scores for %s:\n", hunt_id);
   for(int i = 0; i < index; i++)
      printf("Name: %s, Total: %d\n", array[i].name, array[i].total);
}

int main(int argc, char **argv){
   if(argc != 2){
      perror("Not enough arguments!");
      exit(-1);
   }

   findHuntScore(argv[1]);
   return 0;
}
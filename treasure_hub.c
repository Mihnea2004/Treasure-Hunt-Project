#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define CMD_FILE "monitor_data/command.txt"
pid_t monitor_pid = -1;

void write_command(const char *cmd) {
    int fd = open(CMD_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return;
    }
    write(fd, cmd, strlen(cmd));
    close(fd);
}

void handle_sigchld(int sig) {
    int status;
    waitpid(monitor_pid, &status, 0);
    printf("[hub] Monitor terminated.\n");
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    mkdir("monitor_data", 0755);
    write_command("TEST_COMMAND");

    pid_t pid = fork();
    if (pid == 0)
        _exit(0);
    else if (pid > 0) {
        monitor_pid = pid;
        int status;
        wait(&status);
    } else
        perror("fork");

    return 0;
}

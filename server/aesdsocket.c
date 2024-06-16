#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
volatile sig_atomic_t keep_running = 1;

void handle_signal(int signal) {
    keep_running = 0;
    if (remove("/var/tmp/aesdsocketdata") != 0) {
        perror("Error removing output file");
    }
    exit(0);
}
void handle_client(int client_fd) {
    FILE *fp = fopen("/var/tmp/aesdsocketdata", "a+");
    if (fp == NULL) {
        perror("fopen");
        close(client_fd);
        return;
    }
    syslog(LOG_DEBUG,"Opened file");

    char buffer[1024];
    int bytes_received;
    char *newline_pos;
    int bytes_sent;
    while ((bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        syslog(LOG_DEBUG,"received %s",buffer);


        if ((newline_pos = strchr(buffer, '\n')) != NULL) {
            *newline_pos = '\0';
            fprintf(fp, "%s\n", buffer); 
            break;
            
        } else {
            fprintf(fp, "%s", buffer);
        }
    }
    syslog(LOG_DEBUG,"Wrote to file");

    if (bytes_received == -1) {
        perror("recv");
    }
    rewind(fp);
    while ((bytes_received = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        bytes_sent = send(client_fd, buffer, bytes_received, 0);
        if (bytes_sent == -1) {
            perror("send");
            break;
        }
    }
    syslog(LOG_DEBUG,"Sent data");

    fclose(fp);
    close(client_fd);
}
void daemonize() {
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS); // Parent exits
    }

    if (setsid() < 0) {
        perror("setsid");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS); // Parent exits
    }

    umask(0); // Clear file mode creation mask

    if (chdir("/") < 0) { // Change working directory to root
        perror("chdir");
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
int main(int argc, char **argv){
    
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    openlog(NULL,0,LOG_USER);
    struct addrinfo *result = NULL;
    struct addrinfo hints;
    struct sockaddr their_addr;
    int sockfd,new_fd;
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;
    if (getaddrinfo(NULL,"9000",&hints,&result)!=0){
        return -1;
    }
    if((sockfd=socket(result->ai_family,result->ai_socktype,result->ai_protocol))==-1){
        return -1;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            return -1;
        }
    if (bind(sockfd, result->ai_addr, result->ai_addrlen) == -1) {
        close(sockfd);
        perror("server: bind");
    }
    freeaddrinfo(result);
    int daemon_mode = 0;

    // Check for -d flag
    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = 1;
    }

    if (daemon_mode) {
        daemonize();
    }
    if (listen(sockfd, 10) == -1) {
        perror("listen");
        return -1;
    }
    while(keep_running) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.sa_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        syslog(LOG_DEBUG,"Accepted connection from %s\n",s);

        handle_client(new_fd);
        syslog(LOG_DEBUG,"Closed connection from %s\n",s);

    }
    close(sockfd);
    return 0;
}



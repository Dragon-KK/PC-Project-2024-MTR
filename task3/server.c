/**
 * EE23B135 Kaushik G Iyer
 * 24/05/2023
 * 
 * A server compliant with the given client.c file
 * Handles upto MAX_CONCURRENT_CLIENTS connections concurrently :)
 *
 * Inputs:
 *  port {number}
 *  DIR  {path}
 * 
 * NOTE: Only recognizes files with `.mp3` suffix
 * 
 * Outputs:
 *  logs
 * 
*/ 
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// The number of songs we will be serving 💅
#define SONG_COUNT 3

// 16 is a good number frfr
#define MAX_CONCURRENT_CLIENTS 16

// Power of 2 moment
#define BUFFER_SIZE 1024
#define STREAM_CHUNK_SIZE 1024

#pragma region Business Logix
struct ThreadArgument{
    int* live_handlers_count; // Pointer to integer with the count of live handlers
    pthread_mutex_t* live_handlers_count_mutex; // Mutex for above counter
    pthread_cond_t* live_handler_finish_cond; // Conditoin that main loop waits for

    int client_socket; // Client that is to be handled by the thread
    struct sockaddr_in client_address; // Address of client to be handled

    char** song_paths; // List of paths of songs found
};
void* handle_client(void* args);
#pragma endregion

#pragma region Path Management
bool does_contain_suffix(char* string, char* suffix);
char** get_song_paths_from_dir(char* directory);
void free_song_paths(char** song_paths);
#pragma endregion

#pragma region Options
struct Options{
    int port; // The port on which the server will run on
    char* music_directory; // The directory which contains the musics files
};
void set_options(struct Options* options, int argc, char* argv[]);
#pragma endregion

#include <signal.h>

volatile sig_atomic_t stop;
int server_socket;
void inthand(int signum) { // https://stackoverflow.com/a/54267342
    stop = 1;
    close(server_socket);
}

int main(int argc, char* argv[]){
    signal(SIGINT, inthand);
    struct Options options; set_options(&options, argc, argv);

    char** song_paths = get_song_paths_from_dir(options.music_directory);
    int live_handlers_count = 0;
    pthread_cond_t live_handler_finish_cond;
    pthread_cond_init(&live_handler_finish_cond, NULL);
    pthread_mutex_t live_handlers_count_mutex;
    pthread_mutex_init(&live_handlers_count_mutex, NULL);

    // Create socket to listen on
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1){
        fprintf(stderr, "ERROR! Could not create socket for listening\n");
        exit(1);
    }

    // This was yoinked from the example given
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET; // Since we are dealing with TCP
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Listen on localhost
    server_address.sin_port = htons(options.port); // Converts to big endian if machine uses little endian

    // Tries to bind socket to address
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1){
        fprintf(stderr, "ERROR! Could not bind socket to port\n");
        exit(1);
    }

    // Allow a backlog of 2*... cause why not
    if (listen(server_socket, 2 * MAX_CONCURRENT_CLIENTS) == -1){
        fprintf(stderr, "ERROR! Could not listen on socket\n");
        exit(1);
    }

    // https://www.gta.ufrj.br/ensino/eel878/sockets/inet_ntoaman.html
    printf("Listening on %s:%d\n", inet_ntoa(server_address.sin_addr), options.port);

    stop = 0;
    while(!stop){
        pthread_mutex_lock(&live_handlers_count_mutex);
        while (live_handlers_count >= MAX_CONCURRENT_CLIENTS){ // Wait until we can accept new connections
            pthread_cond_wait(&live_handler_finish_cond, &live_handlers_count_mutex);
        }
        pthread_mutex_unlock(&live_handlers_count_mutex);

        struct ThreadArgument* ta = malloc(sizeof(struct ThreadArgument));
        if (ta == NULL){
            fprintf(stderr, "ERROR! Could not allocate memory to store thread arguments\n");            
            exit(1);
        }

        ta->live_handlers_count_mutex = &live_handlers_count_mutex;
        ta->live_handler_finish_cond = &live_handler_finish_cond;
        ta->live_handlers_count = &live_handlers_count;     
        ta->song_paths = song_paths;   

        socklen_t client_address_size = sizeof(ta->client_address);
        memset(&ta->client_address, 0, client_address_size);

        // Accept connection (This blocks if no request currently exists)        
        ta->client_socket = accept(server_socket, (struct sockaddr*)&(ta->client_address), &client_address_size);
        if (stop){
            close(ta->client_socket);
            break;
        };

        if (ta->client_socket == -1){
            fprintf(stderr, "ERROR! Could not accept connection\n");
            free(ta);
            continue;
        }

        pthread_mutex_lock(&live_handlers_count_mutex);
        ++live_handlers_count;
        printf("Current active connections %d/%d\n", live_handlers_count, MAX_CONCURRENT_CLIENTS);
        pthread_mutex_unlock(&live_handlers_count_mutex);   
        
        // Spawn a thread to handle the connection
        // NOTE: We don't really need to keep track of the threadID since we never need to join the thread (its ok for it to just die if we Ctrl+C out)
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, (void*)ta);
    }

    // We exit the while loop when Ctrl+C
    close(server_socket);
    free_song_paths(song_paths);
    printf("\nStopping server :)\n");
    return 0;
}

#pragma region Business Logix Impl
/**
 * Handles a single client (To be run on a new thread)
 * NOTE: This will free the argument given to it :)
*/
void* handle_client(void* vta){
    // Logs with formatting
    #define client_logf(ostream, message, ...) fprintf(ostream, "[%s:%d] "message"\n", inet_ntoa(ta->client_address.sin_addr), htons(ta->client_address.sin_port), __VA_ARGS__)
    // Logs without formatting
    #define client_log(ostream, message) fprintf(ostream, "[%s:%d] "message"\n", inet_ntoa(ta->client_address.sin_addr), htons(ta->client_address.sin_port))
    
    #define close_client() { \
        close(ta->client_socket); \
        client_log(stdout, "Closed connection with client"); \
        pthread_mutex_lock(ta->live_handlers_count_mutex); \
        --(*ta->live_handlers_count); \
        printf("Current active connections %d/%d\n", *ta->live_handlers_count, MAX_CONCURRENT_CLIENTS); \
        pthread_cond_signal(ta->live_handler_finish_cond); \
        pthread_mutex_unlock(ta->live_handlers_count_mutex); \
        free(vta); \
        return NULL; \
    }

    struct ThreadArgument* ta = vta;
    
    client_log(stdout, "Established connection with client");
    
    char buffer[BUFFER_SIZE]; // Receive the song number requested
    int result = recv(ta->client_socket, buffer, BUFFER_SIZE, 0);
    if (result == 0){
        client_log(stderr, "ERROR! Client closed connection before message was sent");
        close_client();
    }
    if (result < 0){
        client_log(stderr, "ERROR! Could not receive data from client");
        close_client();
    }

    int song_idx = atoi(buffer); // Expect 1 indexed number
    if (!(1 <= song_idx && song_idx <= SONG_COUNT)){
        client_logf(stderr, "ERROR! Client sent invalid song index `%d`", song_idx);
        close_client();
    }
    client_logf(stdout, "Client requested song `%d` (`%s`)", song_idx, ta->song_paths[song_idx - 1]);

    FILE* fd = fopen(ta->song_paths[song_idx - 1], "rb");
    if (fd == NULL){
        client_logf(stderr, "ERROR! Could not open song `%d` (`%s`)", song_idx, ta->song_paths[song_idx - 1]);
        close_client();
    }

    // https://stackoverflow.com/a/65029800
    while (true) { // Stream the song to the client
        size_t num_read = fread(buffer, 1, STREAM_CHUNK_SIZE, fd);
        if (num_read == 0){ // Reached EOF or some error happened lmao
            if (ferror(fd)){ // Check if error happened
                client_log(stderr, "ERROR! Could not read file :)");
            } else{ // No error happened
                client_log(stdout, "Successfully streamed song to client :)");
            }
            fclose(fd);
            close_client();
        }
        ssize_t num_sent = send(ta->client_socket, buffer, num_read, 0);
        if (num_sent < 0){
            client_log(stderr, "ERROR! Could not send data to client");
            fclose(fd);
            close_client();
        }
    }
}
#pragma endregion

#pragma region Path Management Impl
/**
 * Returns true if the string contains the given suffix
 * NOTE: This doesn't ignore whitespace
*/
bool does_contain_suffix(char* string, char* suffix){
    int string_len = strlen(string);
    int suffix_len = strlen(suffix);

    return (string_len >= suffix_len) && (strcmp(string + string_len - suffix_len, suffix) == 0);
}

/**
 * Loads SONG_COUNT number of file paths from the given directory that contain the `.mp3` suffix
 * NOTE: Directory traversal is fucked in C. Please don't put yourself through this shit again
 * RAISES: Exits if invalid directory given
*/
char** get_song_paths_from_dir(char* directory){
    char** song_paths = malloc(SONG_COUNT * sizeof(char*));
    
    int directory_name_size = strlen(directory);

    int idx = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if (d) { // https://stackoverflow.com/a/4204758
        while ((dir = readdir(d)) != NULL && idx < SONG_COUNT) {
            if (does_contain_suffix(dir->d_name, ".mp3")){
                // Enough space to store null terminated string of joint path
                int file_name_size = strlen(dir->d_name);
                song_paths[idx] = malloc(directory_name_size + 1 + file_name_size + 1);

                strcpy(song_paths[idx] + 0, directory);
                strcpy(song_paths[idx] + directory_name_size, "/");
                strcpy(song_paths[idx] + directory_name_size + 1, dir->d_name);
                song_paths[idx][directory_name_size + 1 + file_name_size] = '\0';

                ++idx;
            }
        }
        closedir(d);
    } else{
        fprintf(stderr, "ERROR! Could not open directory `%s`\n", directory);
        exit(1);
    }

    if (idx != SONG_COUNT){
        fprintf(stderr, "ERROR! Was only able to load %d/%d songs from `%s`\n", idx, SONG_COUNT, directory);
        exit(1);
    }

    printf("Found %d songs\n", SONG_COUNT);
    for (int i = 0; i < SONG_COUNT; ++i){
        printf("%d: `%s`\n", i + 1, song_paths[i]);
    }
    return song_paths;
}

/**
 * Frees the memory associated with the list of song_paths
*/
void free_song_paths(char** song_paths){
    for (int i = 0; i < SONG_COUNT; ++i){
        free(song_paths[i]);
    }
    free(song_paths);
}
#pragma endregion

#pragma region Options Impl
/**
 * Sets options based on command line arguments
*/
void set_options(struct Options* options, int argc, char* argv[]){
    options->port = (argc > 1) ? atoi(argv[1]) : 3000;
    options->music_directory = (argc > 2) ? argv[2] : "./media";
}
#pragma endregion

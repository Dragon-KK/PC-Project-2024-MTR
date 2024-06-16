/**
 * EE23B135 Kaushik G Iyer
 * 15/06/2024
 * 
 * Implements a chat server that is compatible with the given client
 * 
 * Inputs:
 *  - PORT <int> The port on which the server should running on
 *  - MAX_CONCURRENT_CLIENTS <int> The number of concurrent clients the server will support
 *  - TIMEOUT <int> The number of seconds of inactivity after which a client will be kicked (-1 to never timout)
 * 
 * Server Commands:
 *  - `\list` Lists out all the users currently logged in
 *  - `\bye` Tells the server to remove the client from the chatroom
*/ 

#include <stdexcept>
#include <iostream>
#include <string>
#include <format>
#include <csignal>
#include <cstring>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <map>

#ifdef WIN32
#include <Winsock2.h>
#include <ws2tcpip.h>
#else
using SOCKET = int;
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

struct Client{
private:
    SOCKET sock;
    sockaddr_in address;
    char buffer[BUFFER_SIZE];

    void clear_buffer();
public:
    bool timed_out;
    int timeout;
    
    std::atomic_bool is_online;
    std::string name;
    
    void send_message(const std::string& message);
    std::string get_message();
    bool wait();
    void close_();
    inline const char* address_name();
    inline const int port();
    inline std::ostream& logger(std::ostream& stream);
    inline std::ostream& logger();
};

struct Options{
private:
    const static int default_port = 8000;
    const static int default_max_concurrent_clients = 16;
    const static int default_timeout = -1;

    void init(int port, int max_concurrent_clients, int timeout){
        this->port = port;
        this->max_concurrent_clients = max_concurrent_clients;
        this->timeout = timeout;
    };

public:
    int port;
    int max_concurrent_clients;
    int timeout;
    
    Options(int argc, char const *argv[]){
        int port                   = argc > 1 ? std::stoi(argv[1]) : default_port;
        int max_concurrent_clients = argc > 2 ? std::stoi(argv[2]) : default_max_concurrent_clients;
        int timeout                = argc > 3 ? std::stoi(argv[3]) : default_timeout;

        if (port <= 0 || max_concurrent_clients <= 0 || (timeout <= 0 && timeout != -1)){
            throw std::invalid_argument("Received invalid options!");
        }

        init(port, max_concurrent_clients, timeout);
    }
};

namespace Server{
    SOCKET sock;
    sockaddr_in address;
    std::map<std::string, Client*> users;

    void init(int port){
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1){
            throw std::runtime_error("Could not create socket!");
        }

        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET; // Since we are dealing with TCP
        address.sin_addr.s_addr = inet_addr(SERVER_IP); // Listen on localhost
        address.sin_port = htons(port); // Converts to big endian if machine uses little endian
    }

    void bind(){
        if (bind(sock, (sockaddr*)&address, sizeof(address)) == -1){
            throw std::runtime_error("Could not bind socket to port!");;
        }
    }

    void listen_(int backlog){
        
        if (listen(sock, backlog) == -1){
            fprintf(stderr, "ERROR! Could not listen on socket\n");
            exit(1);
        }
    }

    bool register_client(Client* client){
        if (users.count(client->name)) return false;
        users[client->name] = client;
        client->is_online = true;
        return true;
    }

    void unregister_client(Client* client){
        client->is_online = false;
        users.erase(client->name);
    }

    void broadcast(const Client* sender, const std::string& message){
        for (auto [_, client]: users){
            if (client != sender){
                client->send_message(message);
            }
        }
    }

    const std::string get_userlist(){
        std::string userlist;
        for (auto [name, _]: users){
            userlist += "\n\t-" + name;
        }
        return userlist;
    }

    inline const char* address_name(){
        return inet_ntoa(address.sin_addr);
    }

    inline const int port(){
        return htons(address.sin_port);
    }

    inline std::ostream& logger(std::ostream& stream){
        return stream << "[SERVER (" << address_name() << ":" << port() << ")] ";
    }
    inline std::ostream& logger(){
        return logger(std::cout);
    }
    
}

namespace sync_tools{
    std::condition_variable accept_ready_cv;
    std::mutex accept_ready_m;

    volatile std::atomic_bool running = true;
    std::atomic_int32_t live_users = 0;

    /**
     * Holds the thread until the server can accept more users or the server is closed
     */
    void wait_till_can_accept(int max_concurrent_clients){
        std::unique_lock accept_read_lk(accept_ready_m);
        accept_ready_cv.wait(accept_read_lk, [max_concurrent_clients](){return (!sync_tools::running) || sync_tools::live_users < max_concurrent_clients;});
        accept_read_lk.unlock();
    }

    /**
     * Function to handle the SIGINT signal
     */
    void handle_sig(int signum) { // https://stackoverflow.com/a/54267342
        sync_tools::running = false;
        sync_tools::accept_ready_cv.notify_all();
        
        #ifdef WIN32
        shutdown(Server::sock, SD_BOTH);
        #else
        shutdown(Server::sock, SHUT_RDWR);
        #endif
    }
}

void Client::clear_buffer(){
    memset(buffer, 0, BUFFER_SIZE);
}
void Client::send_message(const std::string& message){
    send(sock, message.c_str(), message.size(), 0);
}
std::string Client::get_message(){
    clear_buffer();
    timed_out = false;

    #ifndef WIN32
    if (timeout != -1){
        struct pollfd fd;
        int ret;

        fd.fd = sock; // your socket handler 
        fd.events = POLLIN;
        ret = poll(&fd, 1, 1000 * timeout); // 1 second for timeout

        timed_out = ret == 0;
        if (ret <= 0){
            return "";
        }
    }
    #endif

    int result = recv(sock, buffer, BUFFER_SIZE, 0);
    
    if (result == 0){
        logger(std::cerr) << "ERROR! Client closed connection before message was sent" << std::endl;
        return "";
    }
    if (result < 0){
        logger(std::cerr) << "ERROR! Could not receive message from client" << std::endl;
        return "";
    }

    return std::string(buffer);
}
bool Client::wait(){
    socklen_t client_address_size = sizeof(address);
    memset(&address, 0, client_address_size);

    sock = accept(Server::sock, (struct sockaddr*)&(address), &client_address_size);
    return sock != -1;
}
void Client::close_(){
    sync_tools::live_users--;
    sync_tools::accept_ready_cv.notify_all();
    #ifdef WIN32
    closesocket(sock);
    #else
    close(sock);
    #endif
}
inline const char* Client::address_name(){
    return inet_ntoa(address.sin_addr);
}
inline const int Client::port(){
    return htons(address.sin_port);
}
inline std::ostream& Client::logger(std::ostream& stream){
    return stream << "[CLIENT (" << address_name() << ":" << port() << ")] ";
}
inline std::ostream& Client::logger(){
    return logger(std::cout);
}

void handle_client(Client* client){
    client->logger() << "Connection established" << std::endl;
    
    client->send_message(
        "<SERVER> Welcome to the chatroom\n"
        "<SERVER> Currently connected users are:" +
        Server::get_userlist()
    );
    client->name = client->get_message();
    if (!client->name.size()){
        client->close_(); return;
    }
    if (client->timed_out){
        client->send_message("<SERVER> Connection timed out");
        client->close_(); return;
    }

    if (!Server::register_client(client)){
        client->logger(std::cerr) << "Tried to use existing name" << std::endl;
        client->close_();
    }
    
    Server::broadcast(client, "<SERVER> Client " + client->name + " has joined the chatroom");

    while (client->is_online){ // TODO: Timeout
        std::string message = client->get_message();

        if (client->timed_out){
            client->send_message("<SERVER> Connection timed out");
            Server::unregister_client(client);
            continue;
        }

        if (!message.size()){
            Server::unregister_client(client);
            continue;
        }
        if (message == "\\list"){
            client->send_message("<SERVER> Currently connected users are:" + Server::get_userlist());
            continue;
        }
        if (message == "\\bye"){
            client->logger() << "Said goodbye" << std::endl;
            Server::unregister_client(client);
            continue;
        }

        Server::broadcast(client, "[" + client->name + "] " + message);
    }
    Server::broadcast(client, "<SERVER> Client " + client->name + " has left the chatroom");
    
    client->close_();
    delete client;
}


int main(int argc, char const *argv[])
{
    Options options(argc, argv);

    Server::init(options.port);
    Server::bind();
    Server::listen_(options.max_concurrent_clients * 2);

    Server::logger() << "Waiting for connections" << std::endl;

    std::signal(SIGINT, sync_tools::handle_sig);
    while (sync_tools::running){
        sync_tools::wait_till_can_accept(options.max_concurrent_clients);
        if (!sync_tools::running) break;

        Client* client = new Client();
        client->timeout = options.timeout;

        if (!client->wait()){
            Server::logger(std::cerr) << "ERROR! Could not accept connection\n" << std::endl;
            delete client;
            continue;
        }

        sync_tools::live_users++;
        Server::logger() << "Current active connections " << sync_tools::live_users << "/" << options.max_concurrent_clients << std::endl;
        
        if (!sync_tools::running){
            client->close_();
            break;
        };

        std::thread(handle_client, client).detach();
    }

    Server::logger() << "Stopping server" << std::endl;
    for (auto [_, client]: Server::users){
        client->close_();
    }

    #ifdef WIN32
    closesocket(Server::sock);
    #else
    close(Server::sock);
    #endif

    return 0;
}

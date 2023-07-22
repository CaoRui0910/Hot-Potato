#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>

#include "potato.h"

int Potato::get_num_hops(){
    return this->num_hops;
}

void Potato::print_potato_trace(){
    std::cout << "Trace of potato:\n";
    for (int i = 0; i < this->count-1; i++) {
        std::cout << this->trace[i] << ",";
    }
    //????????????????????????????No spaces or newlines in the list.
    std::cout << this->trace[this->count - 1] << "\n";
}

void Potato::decrement_num_hops(){
    this->num_hops--;
}

void Potato::append_id(int player_id){
    this->trace[this->count] = player_id;
    this->count++;
}

//Functions:
/*
The implementstions of server(), accept_client_request(), and client() 
refer to the resource "tcp_example.zip" provided by our professor.
*/
int server(const char * port_num, bool is_no_port){
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = NULL;
    const char *port = port_num;

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        std::cerr << "Error: cannot get address info for host" << "\n";
        std::cerr << "  (" << hostname << "," << port << ")" << "\n";
        exit(EXIT_FAILURE);
    }

    if(is_no_port == true){
        //???????????????????????
        //player: no port->assign port
        struct sockaddr_in *addr = (struct sockaddr_in *)(host_info_list->ai_addr);
        addr->sin_port = 0;
    }

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socket_fd == -1) {
        std::cerr << "Error: cannot create socket" << "\n";
        std::cerr << "  (" << hostname << "," << port << ")" << "\n";
        exit(EXIT_FAILURE);
    } //if

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cerr << "Error: cannot bind socket" << "\n";
        std::cerr << "  (" << hostname << "," << port << ")" << "\n";
        exit(EXIT_FAILURE);
    } //if

    status = listen(socket_fd, 100);
    if (status == -1) {
        std::cerr << "Error: cannot listen on socket" << "\n"; 
        std::cerr << "  (" << hostname << "," << port << ")" << "\n";
        exit(EXIT_FAILURE);
    } //if

    //free objects allocated on the heap: freeaddrinfo() after getaddrinfo()
    freeaddrinfo(host_info_list);
    return socket_fd;
}

//server calls accept() to receive the client's connection request:
int accept_client_request(int socket_fd, std::string &ip){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd;
    client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
        std::cerr << "Error: cannot accept connection on socket" << "\n";
        exit(EXIT_FAILURE);
    } //if
    //?????????????????????
    struct sockaddr_in * sockaddr = (struct sockaddr_in *)&socket_addr;
    ip = inet_ntoa(sockaddr->sin_addr);
    return client_connection_fd;
}

int client(const char * hostname, const char * port_num){
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *port = port_num;

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        std::cerr << "Error: cannot get address info for host" << "\n";
        std::cerr << "  (" << hostname << "," << port << ")" << "\n";
        exit(EXIT_FAILURE);
    } //if

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socket_fd == -1) {
        std::cerr << "Error: cannot create socket" << "\n";
        std::cerr << "  (" << hostname << "," << port << ")" << "\n";
        exit(EXIT_FAILURE);
    } //if
    
    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cerr << "Error: cannot connect to socket" << "\n";
        std::cerr << "  (" << hostname << "," << port << ")" << "\n";
        exit(EXIT_FAILURE);
    } //if

    //free objects allocated on the heap: freeaddrinfo() after getaddrinfo()
    freeaddrinfo(host_info_list);
    return socket_fd;
}


//当send和recv报错时要终止程序吗？????????????????
//send(int socket, const void *buffer, size_t length, int flags);
void mysend(int socket, const void *buffer, size_t length, int flags){
    //send(): If successful, send() returns 0 or greater indicating the number of bytes sent.
    if(send(socket, buffer, length, flags) != length){
        std::cerr << "Error: send()\n";
        exit(EXIT_FAILURE);
    }
}

//recv(int socket, void *buffer, size_t length, int flags);
void myrecv(int socket, void *buffer, size_t length, int flags){
    if(recv(socket, buffer, length, flags) != length){
        std::cerr << "Error: recv()\n";
        exit(EXIT_FAILURE);
    }
}

//The calling of select() refers to man page
//int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
void myselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, timeval *timeout){
    int status = select(nfds, readfds, writefds, exceptfds, timeout);
    //select() return -1 if an error occurred
    if(status < 0){
        std::cerr << "Error: select()\n";
        exit(EXIT_FAILURE);
    }
}

//??????????????????
//Get no specified port:
int get_port(int socket_fd){
    struct sockaddr_in myaddr;
    socklen_t addr_len = sizeof(myaddr);
    int status = getsockname(socket_fd, (struct sockaddr *)&myaddr, &addr_len);
    //On error, -1 is returned:
    if(status == -1){
        std::cerr << "Error: getsockname()" << "\n";
        exit(EXIT_FAILURE);
    }
    return ntohs(myaddr.sin_port);
}
#include <string.h>

class Potato {
    private:
    int num_hops;
    int count;
    int trace[512];

    public:
    Potato() : num_hops(0), count(0) { memset(trace, 0, sizeof(trace)); }
    Potato(int num_hops) : num_hops(num_hops), count(0) { memset(trace, 0, sizeof(trace)); }
    int get_num_hops();
    void print_potato_trace();
    void decrement_num_hops();
    void append_id(int player_id);
};


//Function prototypes:
/*
The implementstions of server(), accept_client_request(), and client() 
refer to the resource "tcp_example.zip" provided by our professor.
*/
int server(const char * port_num, bool is_no_port);

int accept_client_request(int socket_fd, std::string &ip);

int client(const char * hostname, const char * port_num);

int get_port(int socket_fd);

//for send() and recv(), make sure to handle the error conditions and socket closure cases
void mysend(int socket, const void *buffer, size_t length, int flags);
void myrecv(int socket, void *buffer, size_t length, int flags);

void myselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
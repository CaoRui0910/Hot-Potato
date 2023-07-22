#include <cstdlib>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <bits/stdc++.h>

#include "potato.h"

//main function
//ringmaster <port_num> <num_players> <num_hops>
int main(int argc, char * argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: filename\n";
        return EXIT_FAILURE;
    }
    const char *  port_num = argv[1];
    int num_players = atoi(argv[2]);
    int num_hops = atoi(argv[3]);
    //make sure to validate my command line arguments:
    if(!(num_players > 1)){
        std::cerr << "num_players must be greater than 1\n";
        return EXIT_FAILURE;
    }
    if(num_hops < 0 || num_hops > 512){
        std::cerr << "num_hops must be greater than or equal to zero and less than or equal to 512\n";
        return EXIT_FAILURE;
    }

    //Initially (output):
    std::cout << "Potato Ringmaster\n";
    std::cout << "Players = " << num_players << "\n";
    std::cout << "Hops = " << num_hops << "\n";

    std::vector<int> player_fd_list;
    std::vector<std::string> player_ip_list;
    std::vector<int> player_port_list;

    //1. Establish N network socket connections with N number of players 
    //2. provide relevant information to each player (player id & num_players, neighbor's ip & port)
    int socket_fd = server(port_num, false);
    for (int i = 0; i < num_players; i++) {
        std::string player_ip;
        int player_port;
        int player_fd;
        //server (ringmaster) accept clients' (players) connection request:
        player_fd = accept_client_request(socket_fd, player_ip);

        //ringmaster provide relevant information (player id & num_players) to each player
        mysend(player_fd, &i, sizeof(i), 0);
        mysend(player_fd, &num_players, sizeof(num_players), 0);

        //ringmaster receives players' ports:
        //flags??????????????????? MSG_WAITALL???????????????
        myrecv(player_fd, &player_port, sizeof(player_port), MSG_WAITALL);

        player_fd_list.push_back(player_fd);
        player_ip_list.push_back(player_ip);
        player_port_list.push_back(player_port);
        //Upon connection with a player (output):
        std::cout << "Player " << i << " is ready to play\n";
    }

    //ringmaster provide relevant information (neighbor's ip & port) to each player
    //left neighbor:
    for (int i = 0; i < num_players; i++) {
        int left_neighbor_id;
        if(i == 0){
            left_neighbor_id = num_players-1;
        }else{
            left_neighbor_id = i-1;
        }
        //????????????????????????????
        char left_neighbor_ip[100];
        memset(left_neighbor_ip, 0, sizeof(left_neighbor_ip));
        strcpy(left_neighbor_ip, player_ip_list[left_neighbor_id].c_str());
        mysend(player_fd_list[i], &left_neighbor_ip, sizeof(left_neighbor_ip), 0);
        int left_neighbor_port = player_port_list[left_neighbor_id];
        mysend(player_fd_list[i], &left_neighbor_port, sizeof(left_neighbor_port), 0);
    }

    //Play game:
    //3. Create a “potato” object
    Potato potato(num_hops);
    //if num_hops is zero, the ringmaster immediately shuts down the game and no trace of hops is printed.
    if (num_hops == 0) {
        for (int i = 0; i < num_players; i++) {
            mysend(player_fd_list[i], &potato, sizeof(potato), 0);
        }
        close(socket_fd);
        for (int i = 0; i < num_players; i++) {
            close(player_fd_list[i]);
        }
        return EXIT_SUCCESS;
    }

    //4. Randomly select a player and send the “potato” to the selected player
    //????srand((unsigned int)time(NULL)+player_id);??????????????
    srand((unsigned int)time(NULL)+num_players);
    int random_player = rand() % num_players;
    //When launching the potato to the first randomly chosen player (output):
    std::cout << "Ready to start the game, sending potato to player " << random_player << "\n";
    mysend(player_fd_list[random_player], &potato, sizeof(potato), 0);

    //5. End of game: ringmaster receive the potato from the player who is "it"
    fd_set rfds;
    FD_ZERO(&rfds);
    for (int i = 0; i < num_players; i++) {
        FD_SET(player_fd_list[i], &rfds);
    }
    //nfds should be the highest socket descriptor I specified, plus 1
    int nfds = *max_element(player_fd_list.begin(), player_fd_list.end()) + 1;
    //If timeout is a null pointer, the select blocks indefinitely.
    myselect(nfds, &rfds, NULL, NULL, NULL);
    for (int i = 0; i < num_players; i++) {
        if (FD_ISSET(player_fd_list[i], &rfds)) {
            //??????????????????????MSG_WAITALL????????
            myrecv(player_fd_list[i], &potato, sizeof(potato), MSG_WAITALL);
        }
    }

    //6. Shut the game down by sending a message (potato with 0 num_hops) to each player
    for (int i = 0; i < num_players; i++) {
        mysend(player_fd_list[i], &potato, sizeof(potato), 0);
    }
    //at the end of the game, print trace of potato (output):
    potato.print_potato_trace();

    //7. game over: all processes must close
    close(socket_fd);
    for (int i = 0; i < num_players; i++) {
        close(player_fd_list[i]);
    }
    return EXIT_SUCCESS;
}
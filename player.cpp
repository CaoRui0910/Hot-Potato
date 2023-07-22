#include <cstdlib>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <bits/stdc++.h>

#include "potato.h"
//main
//player <machine_name> <port_num>
int main(int argc, char * argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: filename\n";
        return EXIT_FAILURE;
    }
    const char * machine_name = argv[1];
    const char * port_num = argv[2];

    //player connect to ringmaster
    int ringmaster_fd = client(machine_name, port_num);

    //receive player id and num_players from ringmaster
    int player_id;
    int num_players;
    //???????????????????????????????????
    myrecv(ringmaster_fd, &player_id, sizeof(player_id), MSG_WAITALL);
    myrecv(ringmaster_fd, &num_players, sizeof(num_players), MSG_WAITALL);

    //player send port to ringmaster
    int player_fd = server("", true);
    int player_port = get_port(player_fd);
    mysend(ringmaster_fd, &player_port, sizeof(player_port), 0);

    //player receive left neighbor's ip and port from ringmaster
    //????????????????????????????????????????
    char left_neighbor_ip[100];
    //????????????????????????????????????????
    //memset(left_neighbor_ip, 0, sizeof(left_neighbor_ip));
    int left_neighbor_port;
    //????????????????????????????????????????
    myrecv(ringmaster_fd, &left_neighbor_ip, sizeof(left_neighbor_ip), MSG_WAITALL);
    myrecv(ringmaster_fd, &left_neighbor_port, sizeof(left_neighbor_port), MSG_WAITALL);

    //After receiving initial message (num_players, info about neighbors) from ringmaster (output):
    std::cout << "Connected as player " << player_id << " out of " << num_players << " total players\n";
    //player (client) connect to left neighbor (server)
    //???????????????????????????????????????????
    char left_neighbor_port1[9];
    sprintf(left_neighbor_port1, "%d", left_neighbor_port);
    int left_neighbor_fd = client(left_neighbor_ip, left_neighbor_port1);
    //player (server) accept connection request of right neighbor (client)
    std::string right_neighbor_ip;
    int right_neighbor_fd = accept_client_request(player_fd, right_neighbor_ip);

    //Play game:
    //????????????????????????????????????????
    srand((unsigned int)time(NULL) + player_id);
    //1. player receive the potato from three connections (left neighbor or right or ringmaster)
    fd_set rfds;
    std::vector<int> three_connections_fd;
    three_connections_fd.push_back(left_neighbor_fd);
    three_connections_fd.push_back(right_neighbor_fd);
    three_connections_fd.push_back(ringmaster_fd);
    Potato potato;
    while (true) {
        FD_ZERO(&rfds);
        for (int i = 0; i < 3; i++) {
            FD_SET(three_connections_fd[i], &rfds);
        }
        int nfds = *max_element(three_connections_fd.begin(), three_connections_fd.end()) + 1;
        myselect(nfds, &rfds, NULL, NULL, NULL);
        int status;
        for (int i = 0; i < 3; i++) {
            if (FD_ISSET(three_connections_fd[i], &rfds)) {
                //???????????????????????????????????
                status = recv(three_connections_fd[i], &potato, sizeof(potato), MSG_WAITALL);
                break;
            }
        }
        //2. If player receive a potato with 0 num_hops, it means game over
        //recv(): The value 0 indicates the connection is closed. So if the status is 0, 
        //it means connection is break (other socket is closed) -> game over
        if(potato.get_num_hops() == 0 || status == 0){
            break;
        }
        //player decrement the number of hops and append the player’s ID
        potato.decrement_num_hops();
        potato.append_id(player_id);
        if(potato.get_num_hops() > 0){
            //3. If the remaining number of hops is greater than zero, 
            //the player will randomly select a neighbor and send the potato to that neighbor.
            int randnum_neighbor = rand() % 2;
            mysend(three_connections_fd[randnum_neighbor], &potato, sizeof(potato), 0);
            //When forwarding the potato to another player (output):
            if(randnum_neighbor == 0){ 
                //left neighbor
                int left_neighbor_id;
                if(player_id == 0){
                    left_neighbor_id = num_players - 1;
                }else{
                    left_neighbor_id = player_id-1;
                }
                std::cout << "Sending potato to " << left_neighbor_id << "\n";
            }else{ 
                //right neighbor
                int right_neighbor_id;
                if(player_id == num_players - 1){
                    right_neighbor_id = 0;
                }else{
                    right_neighbor_id = player_id+1;
                }
                std::cout << "Sending potato to " << right_neighbor_id << "\n";
            }
        }else if (potato.get_num_hops() == 0){
            //4. If player is the potato with the last hop, player will sand potato to ringmaster
            mysend(ringmaster_fd, &potato, sizeof(potato), 0);
            //When number of hops is reached (output):
            std::cout << "I'm it" << "\n";
        }
    }
    //5. The game ends and all processes must close
    close(ringmaster_fd);
    close(left_neighbor_fd);
    close(right_neighbor_fd);
    return EXIT_SUCCESS;
}
# Hot-Potato
Applied C++, TCP Socket Programming to implement Hot Potato game.
### Introduction
- Create one ringmaster process and some number of player processes, then play a game and terminate all the processes gracefully
- Use TCP sockets as the mechanism for communication between the ringmaster and player processes
- Game Flow
  - Initially, the server will set the port number and determine the number of players participating in the game, as well as the predefined number of times the potato will be passed.
  - Subsequently, each player will connect to the server.
  - The server will initiate the game by passing the potato to a randomly chosen player.
  - Then, the potato will continue to be passed either to the previous player or the next player until the specified number of passes is reached.
  - Finally, the server will display the trace of the potato's journey and the last player who received the potato will provide some information or perform a specific action.

### How to play the game?
- The server program is invoked as:
  ```
  ringmaster <port_num> <num_players> <num_hops>
  (example: ./ringmaster 1234 3 100)
  ```
- The player program is invoked as:
  ```
  player <machine_name> <port_num>
  (example: ./player vcm-xxxx.vm.duke.edu 1234)
  ```

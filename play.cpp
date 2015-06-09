#include "game.h"
#include <sstream>
#include <string>
#include <iostream>
#include <map>

using namespace std;

int main(int argc,char **argv)
{
    freopen("game","a",stdout); 
    string client_ip = string(argv[3]);
    string server_ip = string(argv[1]);
    int server_port = atoi(argv[2]);
    int client_port = atoi(argv[4]);
    string client_id = string(argv[5]);
    Game new_game(server_ip,server_port,client_ip,client_port,client_id);
    new_game.start_game();

	return 0;
}

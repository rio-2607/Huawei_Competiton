#ifndef GAME_H
#define GAME_H
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <sys/types.h>	/* basic system data types */
#include <sys/socket.h>	/* basic socket definitions */
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>	/* inet(3) functions */
#include <errno.h>
#include <fcntl.h>		/* for nonblocking */
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>	/* for S_xxx file mode constants */
#include <sys/uio.h>		/* for iovec{} and readv/writev */
#include <unistd.h>
#include <sys/wait.h>
#include <sys/un.h>		/* for Unix domain sockets */
#include <cmath>
using namespace std;

enum card_color{SPADES,HEARTS,CLUBS,DIAMONDS}; //The color of card

typedef struct Card {
    card_color color;//
    int point;
}Card;

// add a new choice named None,It means that the game is not start
enum choice {CHECK,CALL,RAISE,ALL_IN,FOLD,NONE_CHOICE};//the operation choice

//current state of each player，including in,out,NONE means that the game is not start
enum player_state {OUT,IN,NONE_PlAYER_STATE};

enum game_stage{PRE_FLOP,FLOP,TURN,RIVER,NONE_GAME_STATE};//the stage that we may encountered
typedef struct Player {
    int remain_money;//remaining money
    int jetton_on_hand;//remaining jetton 
    int bet;
    enum choice action;//the last operation
	vector<enum choice> historical_operation;//record the historical operations of player
    enum player_state current_state;//the current state
    string pid;//ID of the player
    Card cards[2];//cards on hand
}Player;

class Game
{
public:
    Game();
    ~Game(){}
    Game(string s_ip,int s_port,string m_ip,int m_port,string m_id):server_ip(s_ip),
        server_port(s_port),my_ip(m_ip),my_port(m_port),my_id(m_id),stage(NONE_GAME_STATE),
        total_money(0),my_index(0),current_bet(0),hold_rank(0),blind_bet(0),num_in_game(8),num_of_hand(0)
    {
        //init the color_table and point_table
        color_table["SPADES"] = SPADES;
        color_table["HEARTS"] = HEARTS;
        color_table["CLUBS"] = CLUBS;
        color_table["DIAMONDS"] = DIAMONDS;

        for(int i = 2;i <= 10;++i)
        {
            stringstream ss;
            ss << i;
            string index;
            ss >> index;
            point_table[index] = i;
        }
        point_table["J"] = 11;
        point_table["Q"] = 12;
        point_table["K"] = 13;
        point_table["A"] = 14;


        actions_table["check"] = CHECK;
        actions_table["call"] = CALL;
        actions_table["raise"] = RAISE;
        actions_table["all_in"] = ALL_IN;
        actions_table["fold"] = FOLD;
    }

private:
	int num_in_game;
	int blind_bet;
	int current_bet;
    int total_money;//total money in the pot
    int my_index;//my position among the players,the first seat represents button
	int hold_rank;
	int wealth_rank;
	int num_of_hand;
    vector<Player> players;//information of all players,the first button,second small blind ,an so on...
    map<string,int> index_map;//key:PID of each player，value:the index of player whose ID is PID
    Card public_cards[5];//three public cards,one is turn card ，one is river card
    enum game_stage stage;//current game stage


    string server_ip;//the IP address of server
    int server_port;//the port of server
    string my_ip;
    int my_port;
    string my_id;

    map<string,card_color> color_table;//color stable
    map<string,int> point_table;//points table
    map<string,choice> actions_table;//actions table
public:
    void start_game();//start game 
private:
    //init the players vector and index_map,call this function when u receive
    //the seat-info-msg,the parameter is the msg that u received
    void init_players(const string &msg);
    //this function is the really function that begin the game,it was called ny start_game()
    void __start_game();
    void char_2_string(string &msg,char *message)
    {
        msg = message;
    }
	int  message_map(char* recvline,int sockfd);//analyze the received message,and determine which replaying function to be called
	void update_players_info();//update the information of each play
	choice take_action(int &);//replay to the inquire of server
    /*
     * then we waiting for the answer from server.once we receive message form server,
     * first,we should change the messag's type from char* to string.then we begin to analyze the message.if the message's type is
     * seat-info-message,we will call the init_players() function to init all the value we need,else we should call other function
     * ,which will be implemented later,to do some other action.
     */
	
    void get_hold_card_msg(const string &msg);//get the hold-card-message
    void get_inquire_msg(const string &msg);//get the inquire-msg
    void get_blind_msg(const string &msg);//get the blind-msg
    void get_flop_msg(const string &msg);//get flop-msg
    void get_turn_msg(const string &msg);//get turn-msg
    void get_river_msg(const string &msg);//get river-msg
    //get the message body from a string.we set it as a inline function
    void __get_message_body(string &msg_body,const string &msg)
    {
        string::size_type colon_index = msg.find(":");
        if(colon_index != string::npos)//if there is a ":" in line,we should stripped the content before colon
        {
            msg_body = msg.substr(colon_index+1);
        }
        else
            msg_body = msg;
    }
	string choice_to_string(const choice& rep_c,int raise_num)
	{
		string rep_s;
		switch (rep_c)
		{
		case CHECK:
			rep_s = "check \n";
			break;
		case CALL:
			rep_s = "call \n";
			break;
		case RAISE:
			{
				stringstream st;
				st << "raise " << raise_num << "\n";
				rep_s = st.str();
				break;
			}
		case ALL_IN:
			rep_s = "all_in \n";
			break;
		default:
			rep_s = "fold \n";
			break;
		}
		return rep_s;
	}
	void get_hold_rank();
	float calc_prob();
};

#endif // GAME_H

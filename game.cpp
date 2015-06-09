#include "game.h"
#include <sstream>
#include <iostream>
#include <cassert>
using namespace std;

#define MAXLINE 4096



int Game::message_map(char* recvline,int sockfd)
{
	int is_close = 0;
	string s(recvline);
	string::size_type pos1,pos2;
	if((pos1 = s.find("seat/")) != string::npos)//if we accept seat information
	{
		pos2 = s.find("/seat");
		string to_treat(s.begin()+pos1,s.begin()+pos2+6);
		init_players(to_treat);
		s = s.substr(pos2+6);
	}
	if((pos1 = s.find("blind/")) != string::npos)//if we accept hold cards information
	{
		pos2 = s.find("/blind");
		string to_treat(s.begin()+pos1,s.begin()+pos2+7);
		get_blind_msg(to_treat);
		s = s.substr(pos2+7);
	}
	if((pos1 = s.find("hold/")) != string::npos)//if we accept hold cards information
	{
		pos2 = s.find("/hold");
		string to_treat(s.begin()+pos1,s.begin()+pos2+6);
		get_hold_card_msg(to_treat);
		s = s.substr(pos2+6);
	}
	if((pos1 = s.find("inquire/")) != string::npos)//if we accept inquire information
	{
		pos2 = s.find("/inquire");
		string to_treat(s.begin()+pos1,s.begin()+pos2+9);
		get_inquire_msg(to_treat);
		int raise_num = 0;
		choice reply_choice = take_action(raise_num);
		string s_rep = choice_to_string(reply_choice,raise_num);
		const char* reply = s_rep.c_str();
		send(sockfd,reply,strlen(reply),0);
		s = s.substr(pos2+9);
	}
	if((pos1 = s.find("flop/")) != string::npos)//if we accept flop card information
	{
		pos2 = s.find("/flop");
		string to_treat(s.begin()+pos1,s.begin()+pos2+6);
		get_flop_msg(to_treat);
		s = s.substr(pos2+6);
	}
	if((pos1 = s.find("turn/")) != string::npos)//if we accept turn card information
	{
		pos2 = s.find("/turn");
		string to_treat(s.begin()+pos1,s.begin()+pos2+6);
		get_turn_msg(to_treat);
		s = s.substr(pos2+6);
	}
	if((pos1 = s.find("river/")) != string::npos)//if we accept river card information
	{
		pos2 = s.find("/river");
		string to_treat(s.begin()+pos1,s.begin()+pos2+7);
		get_river_msg(to_treat);
		s = s.substr(pos2+7);
	}
	if(s.find("game-over") != string::npos)//if we accept game_over information
	{
		is_close = 1;
	}
	return is_close;
}

/*
 * init the value of all the players according to the seat-info-msg
 */
void Game::init_players(const string &msg)
{
    //every time we call the init_players(),we shold clear the infomation that last game leaved
    players.clear();
    index_map.clear();
	num_of_hand++;
    stringstream ss_msg(msg);
    string line;
    int player_index = 0;
	current_bet = 0;
    while(getline(ss_msg,line))
    {
        string::size_type index = line.find("seat");
        //if there is no "seat" in line,it means that this line is not the msg head or msg tail
        // also it contains the message
        if(index == string::npos)
        {
            string msg_body;
            __get_message_body(msg_body,line);//get the message body
            Player player;
            istringstream body(msg_body);
            body >> player.pid >> player.jetton_on_hand >> player.remain_money;
            player.action = NONE_CHOICE;
            player.current_state = IN;
            player.bet = 0;
            players.push_back(player);
            index_map.insert(pair<string,int>(player.pid,player_index));
            if(player.pid == my_id)
                my_index = player_index;//record my position in players vector
            ++player_index;
        }
    }
	num_in_game = players.size();
}

choice Game::take_action(int& raise_num)//***replay to the inquire of server
{
	choice rep_c = NONE_CHOICE;
	Player me = players.at(my_index);
	int least_num = current_bet - me.bet;
	if(num_in_game >= 3)
	{
		hold_rank = hold_rank - 1;
	}
	if(stage == PRE_FLOP)
	{
		if(hold_rank == 5)
		{
			if(least_num > me.jetton_on_hand)
				rep_c = ALL_IN;
			else if(me.jetton_on_hand - least_num <= 4*blind_bet)
				rep_c = CALL;
			else 
			{
				rep_c = RAISE;
				raise_num = (me.jetton_on_hand - least_num - 4*blind_bet)/2;
			}
		}
		else if(hold_rank == 4)
		{
			if(current_bet <= 5*blind_bet)
				if((0 == my_index && num_in_game == 3) || num_in_game == 2)
			{
				rep_c = RAISE;
				raise_num = 1;
			}
			else if(me.bet >= current_bet)
				rep_c = CHECK;
			else if(current_bet >= 20*blind_bet)
				rep_c = FOLD;
			else
				rep_c = CHECK;
		}
		else if(hold_rank == 3 || hold_rank == 2)
		{
			if(me.bet >= current_bet)
				rep_c = CHECK;
			else if(least_num <= 4*blind_bet && (me.jetton_on_hand - least_num) > 20*blind_bet )
				rep_c = CALL;
			else if(num_in_game <= 2 && current_bet - me.bet <= 20*2)
				rep_c = CALL;
			else
				rep_c = CHECK;
		}
		else if(hold_rank == 1)
		{
			if(current_bet <= 4*blind_bet && me.jetton_on_hand >= 8*blind_bet)
				rep_c = CALL;
			else if(least_num <= 0)
				rep_c = CHECK;
			else if(num_in_game <= 2 && current_bet - me.bet <= 20*2)
				rep_c = CALL;
			else
				rep_c = FOLD;
		}
		else 
		{
			if(least_num <= 0)
				rep_c = CHECK;
			else if(me.bet == blind_bet && current_bet == 2*blind_bet  && ((me.cards[0].point >= 8 && me.cards[1].point >= 8)||(me.cards[0].point >= 12)||(me.cards[1].point >= 12)))
				rep_c = CALL;
			else
				rep_c = FOLD;
		}
	}
	else if(stage == FLOP)
	{
		float win_prob = calc_prob();
		if(win_prob < 0.25)
		{
			if( 0 == least_num )
				rep_c = CHECK;
			else
				rep_c = FOLD;
		}
		else if(win_prob > 0.8)
			rep_c = ALL_IN;
		else if(0.8 > win_prob && win_prob > 0.6)
		{
			if(me.remain_money == 0 && (me.jetton_on_hand -least_num) < 4*blind_bet)
				rep_c = FOLD;
			else if(me.bet >= current_bet)
				rep_c = CHECK;
			else
			{
				float expect = win_prob*total_money+(1-win_prob)*least_num;
				if(expect > 0)
					rep_c = CALL;
				else
					rep_c = FOLD;
			}
		}
		else if(0.6 > win_prob && win_prob > 0.25)
		{
			if(me.remain_money > 0 && (me.jetton_on_hand-least_num) > 20*blind_bet)
			{
				float expect = win_prob*total_money+(1-win_prob)*least_num;
				if(expect > 0)
					rep_c = CALL;
				else
					rep_c = FOLD;
			}
			else if(me.bet >= current_bet)
				rep_c = CHECK;
			else
				rep_c = FOLD;
		}
		else
		{
			if(me.bet >= current_bet)
				rep_c = CALL;
			else
				rep_c = CHECK;
		}
	}
	else
		rep_c = CHECK;
	return rep_c;
}

void Game::__start_game()
{
	int sockfd;
    char recvline[MAXLINE+1];
	memset(recvline,0,sizeof(recvline));//whether or not to clear 0
    struct sockaddr_in servaddr;//server IP and port
	struct sockaddr_in clientaddr;//client IP and port
    //creat socket
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        cout << "socket error" << endl;
        exit(0);
    }
	//bind local address to socket
	bzero(&clientaddr,sizeof(clientaddr));
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(my_port);
	//inet_pton-convert dotted decimal string to IP address
    if(inet_pton(AF_INET,my_ip.c_str(),&clientaddr.sin_addr) <= 0)
	{
		cout << "inet_pton error for" << my_ip << endl;
		exit(0);
	}
	int rep = 1;
	setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &rep, sizeof(rep) );
	while(bind(sockfd,(sockaddr*)&clientaddr,sizeof(clientaddr)) < 0 );
    //initialize server information
    bzero(&servaddr,sizeof(servaddr));

    servaddr.sin_family = AF_INET;//address family AF_INET
    servaddr.sin_port = htons(server_port); //htons:convert from host to net short int

    //inet_pton:convert from dotted decimal to integer
    if(inet_pton(AF_INET,server_ip.c_str(),&servaddr.sin_addr) < 0)
    {
        cout << "inet_pton error for" << server_ip << endl;
        exit(0);
    }

    while(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0);
    //send register information to server
    string res_info("reg: ");
    string name(" challenger");
    res_info += my_id +  name + " \n";
    const char *reg = res_info.c_str();
	//send register information
	if(send(sockfd,reg,strlen(reg),0)<0)
	{
		cout << "send register information error" << endl;
		exit(0);
	}
	//wait for server
//	int is_close = 0;//a flag to sign whether to close socket
    int n;
	while(1)
	{
		if((n = read(sockfd,recvline,MAXLINE)) > 0)
		{
			int is_close = message_map(recvline,sockfd);
			memset(recvline,0,sizeof(recvline));//whether or not to clear 0
			if(is_close)
			{
				close(sockfd);
				exit(0);
			}
		}
		if(n < 0)
		{
			cout << "read error";
			exit(0);
		}
	}
}

void Game::start_game()
{
    __start_game();
}



void Game::get_hold_card_msg(const string &msg)
{
    istringstream ss_msg(msg);
    string line;
    int index = 0;
    Player& me = players.at(my_index);
    while(getline(ss_msg,line))
    {
        if(line.find("hold") == string::npos)//if the line is not message tail or head
        {
            if(index < 2)
			{
				istringstream body(line);
				string color;
				body >> color;
				me.cards[index].color = color_table[color];
				string point_str;
				body >> point_str;
				me.cards[index].point = point_table[point_str];
	//            ss_msg > me.cards[index].point;
				++index;
			}
        }
    }
	stage = PRE_FLOP;
	get_hold_rank();
}


void Game::get_inquire_msg(const string &msg)
{
    istringstream ss_msg(msg);
    string line;
    while(getline(ss_msg,line))
    {
        if(line.find("inquire") == string::npos)
        {
            if(line.find(":") != string::npos)//it means that this line looks like total pot:num eol
            {
                string msg_body;
                __get_message_body(msg_body,line);
                istringstream ss(msg_body);
                ss >> total_money;
            }
            else
            {
                istringstream body(line);
                string player_id;
                body >> player_id;
                //in this code,we assume that the player's id is in the map
                //and in general condition,the player's id is in the map
                int player_index = index_map[player_id];
                Player &player = players.at(player_index);//& reference symbol here is necessary

                body >> player.jetton_on_hand;
                body >> player.remain_money;
                body >> player.bet;
                string action;
                body >> action;
                player.action = actions_table[action];
                player.historical_operation.push_back(player.action);
                //if we receive the message of this player,it means that the player is still in game
				if(player.action == FOLD)
				{
					player.current_state = OUT;
				}
				if(player.bet > current_bet)
					current_bet = player.bet;
//                player.
            }
        }
    }
	wealth_rank = 1;
	for(int i = 0;i != players.size();i++)
	{
		int my_wealth = players[my_index].remain_money + players[my_index].jetton_on_hand;
		if(players[i].action != FOLD)
		{
			num_in_game++;
			if(players[my_index].remain_money + players[my_index].jetton_on_hand >= my_wealth)
				wealth_rank++;
		}
	}
}

void Game::get_blind_msg(const string &msg)
{
	istringstream ss_msg(msg);
	string line;
	int index = 0;
	while(getline(ss_msg,line))
	{
		if(line.find("blind") == string::npos)
		{
			    string::size_type colon_index = line.find(":");
				string message_head(line.begin(),line.begin()+colon_index);
				string message_tail(line.begin()+colon_index+1,line.end());
				istringstream ss(message_head);
				string player_id;
				ss >> player_id;
				int player_index = index_map[player_id];
				Player &player = players.at(player_index);//& reference symbol here is necessary
				istringstream body(message_tail);
				body >> player.bet;
				if(index == 0)
					blind_bet = player.bet;
				player.current_state = IN;
				if(player.bet > current_bet)
					current_bet = player.bet;
				index++;
		}
	}
}

void Game::get_flop_msg(const string &msg)
{
    istringstream ss_msg(msg);
    string line;
    int index = 0;

    while(getline(ss_msg,line))
    {
        if(line.find("flop") == string::npos)//if the line is not message tail or head
        {
            if(index < 3)
			{
				string color;
				ss_msg >> color;
				public_cards[index].color = color_table[color];
				string point_str;
				ss_msg >> point_str;
				public_cards[index].point = point_table[point_str];
	//            ss_msg > me.cards[index].point;
				++index;
			}
        }
    }
    stage = FLOP;
}

void Game::get_turn_msg(const string &msg)
{
    istringstream ss(msg);
    string head,body,tail;
    ss >> head >> body >> tail;
    istringstream body_msg(body);
    string color;
    body_msg >> color;
    public_cards[3].color = color_table[color];
    string point_str;
    body_msg >> point_str;
    public_cards[3].point = point_table[point_str];
    stage = TURN;

}


void Game::get_river_msg(const string &msg)
{
    istringstream ss(msg);
    string head,body,tail;
    ss >> head >> body >> tail;
    istringstream body_msg(body);
    string color;
    body_msg >> color;
    public_cards[4].color = color_table[color];
    string point_str;
    body_msg >> point_str;
    public_cards[4].point = point_table[point_str];
    stage = RIVER;
}

void Game::get_hold_rank(void)
{
	int rank = 0;
	Player me = players.at(my_index);
	bool is_same_color = (me.cards[0].color == me.cards[1].color);
	int p0 = me.cards[0].point;
	int p1 = me.cards[1].point;
	if(p0 == p1)
	{
		if(p0 >= 13)
			rank = 5;
		else if(12 >= p0 && p0 >= 10)
			rank = 4;
		else if(p0 >= 8 && p0 <= 9)
			rank = 3;
		else if(p0 >= 7 && p0 <= 6)
			rank = 1;
		else 
			rank = 0;
	}
	else if((13 == p0 && 14 == p1) || (14 == p0 && 13 == p1)|| (12 == p0 && 14 == p1) || (14 == p0 && 12 == p1))
		rank = 3;
	else if((11 == p0 && 14 == p1) || (14 == p0 && 11 == p1)|| (10 == p0 && 14 == p1) || (10 == p0 && 14 == p1))
		rank = 2;
	else if((13 == p0 && 12 == p1) || (12 == p0 && 13 == p1)|| (13 == p0 && 11 == p1) || (11 == p0 && 13 == p1))
		rank = 2;
	else if(p0 >= 10 && p1 >= 10)
		rank =1;
	else
		rank = 0;
	hold_rank = rank;
}

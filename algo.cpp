//Author:MengLu Zhou
#include "game.h"
#include <iostream>
using namespace std;

int compare(const void *a,const void *b) 
{ 
	return ((*((int *)b))-(*((int *)a)));    
} 

void get_rank_value(int *cards,int *value,bool same_color) 
{
	int rank;
	qsort(cards,5,sizeof(int),&compare); 
	int count[15];
	memset(count,0,sizeof(count)); //clear 0
	//record the appearance times of each number 
	for(int j=0;j<5;j++) 
		count[cards[j]]++; 
	rank=0; //rank initialized to be 0 
	//card type analysis
	for(int j=14;j>=2;j--) 
	{ 
		//if there exist repetitive card
		if(count[j]>1)      
		{ 
			switch(count[j]) 
			{  
			case 2:     
				switch(rank) 
				{ 
				case 0: rank=1; value[1]=j; break; //first pair
				case 1: rank=2; value[2]=j; break; //second pair 
				case 3: rank=6; break; //already exist three of a pair          
				}           
				break; 
			case 3: 
				if(0==rank) rank=3;//only three of a pair
				else 
				{ 
					//exist a full house
					rank=6;     
					//record the number of full house  
					value[2] = value[1];
					value[1] = j; 
				} 
				break; 
			case 4: 
				//exist a pair of four 
				rank=7; 
				value[1]=j; 
				break; 
			}                    
		}
	}
	bool is_flush = 1;
	if(rank == 0) 
	{  

		for(int j=1;j<5;j++) 
			if(cards[j]!=cards[j-1]-1) 
			{ 
				is_flush = 0; 
				break;                                      
			} 
			if(is_flush)
		 {
			 if(same_color)
				 rank = 8;
			 else
				 rank = 4;
		 }
	} 
	if(rank < 5 && same_color)
		rank = 5;
	if((rank==4)||(rank==8)) 
	{ 
		value[1]=cards[0];                         
	} 
	if((rank==0)||(rank==5)) 
	{ 
		for(int j=0;j<5;j++) 
		{ 
			value[j+1]= cards[j];                
		}                        
	} 
	if(rank==1) 
	{  
		int k=2; 
		for(int j=0;j<5;j++) 
		{ 
			if(cards[j]!=value[1]) 
				value[k++]=cards[j];                
		}           
	} 
	if(rank==2) 
	{ 
		int k=3; 
		for(int j=0;j<5;j++) 
		{ 
			if(cards[j]!=value[1]) 
			{ 
				if(cards[j]!=value[2]) 
				{ 
					value[k++]=cards[j];                               
				}                              
			}                
		}           
	} 
	value[0]=rank;  
}                

int comp_rank(int *my_rank,int *other_rank)
{
	int rst;
	int i = 0;
	while(i <= 5)
	{
		if(my_rank[i] == other_rank[i])
			i++;
		else
			return my_rank[i] - other_rank[i];
	}
	return my_rank[i] - other_rank[i];
}


float Game::calc_prob()
{
	Player me = players.at(my_index);
	bool same_color_public = public_cards[0].color && public_cards[1].color;
	float prob = 0.0;
	if(!same_color_public)
	{
		int five_cards[5];
		int my_rank_info[6];
		five_cards[0] = public_cards[0].point;
		five_cards[1] = public_cards[1].point;
		five_cards[2] = public_cards[2].point;
		five_cards[3] = me.cards[0].point;
		five_cards[4] = me.cards[1].point;
		get_rank_value(five_cards,my_rank_info,0);
		int count[15];
		memset(count,0,sizeof(count));
		count[me.cards[0].point]++;
		count[me.cards[1].point]++;
		count[public_cards[0].point]++;
		count[public_cards[1].point]++;
		count[public_cards[2].point]++;
		int higher_than_me_num = 0;
		for(int i = 2;i <= 14;i++)
		{
			for(int j = 2;j <= 14;j++)
			{
				int num = 0;
				if(i == j)
				{
					switch (count[i])
					{
					case 0:
						num = 12;
						break;
					case 1:
						num = 6;
						break;
					case 2:
						num = 2;
						break;
					default:
						num = 0;
						break;
					}
				}
				if(i != j)
					num = (4-count[i]) * (4-count[j]);
				if(num == 0)
					continue;
				five_cards[0] = public_cards[0].point;
				five_cards[1] = public_cards[1].point;
				five_cards[2] = public_cards[2].point;
				five_cards[3] = i;
				five_cards[4] = j;
				int other_rank_info[6];
				get_rank_value(five_cards,other_rank_info,0);
				int comp_rst = comp_rank(my_rank_info,other_rank_info);
				if(comp_rst < 0)
					higher_than_me_num += num;
			}
		}
		prob = pow((1.0 - (float)higher_than_me_num / 2162.0),num_in_game);
	}
	if(same_color_public)
	{
		bool same_color_me = ((public_cards[0].color == me.cards[0].color) && (public_cards[0].color == me.cards[1].color));
		if(same_color_me)
		{
			int count[15];
			memset(count,0,sizeof(count));
			count[me.cards[0].point]++;
			count[me.cards[1].point]++;
			count[public_cards[0].point]++;
			count[public_cards[1].point]++;
			count[public_cards[2].point]++;
			int my_rank_info[6];
			int five_cards[5];
			five_cards[0] = public_cards[0].point;
			five_cards[1] = public_cards[1].point;
			five_cards[2] = public_cards[2].point;
			five_cards[3] = me.cards[0].point;
			five_cards[4] = me.cards[1].point;
			get_rank_value(five_cards,my_rank_info,0);
			if(my_rank_info[0] == 8)
			{
				prob = 1.0;
			}
			else
			{
				int higher_than_me_num = 57;
				higher_than_me_num += (14 - five_cards[0]);
				prob = pow(1.0-(float)higher_than_me_num/1080.0,num_in_game);
			}
		}
		if(!same_color_me)
		{
			int five_cards[5];
			int my_rank_info[6];
			five_cards[0] = public_cards[0].point;
			five_cards[1] = public_cards[1].point;
			five_cards[2] = public_cards[2].point;
			five_cards[3] = me.cards[0].point;
			five_cards[4] = me.cards[1].point;
			get_rank_value(five_cards,my_rank_info,0);
			int count[15];
			memset(count,0,sizeof(count));
			count[me.cards[0].point]++;
			count[me.cards[1].point]++;
			count[public_cards[0].point]++;
			count[public_cards[1].point]++;
			count[public_cards[2].point]++;
			int higher_than_me_num = 0;
			for(int i = 2;i <= 14;i++)
			{
				for(int j = 2;j <= 14;j++)
				{
					int num = 0;
					if(i == j)
					{
						switch (count[i])
						{
						case 0:
							num = 12;
							break;
						case 1:
							num = 6;
							break;
						case 2:
							num = 2;
							break;
						default:
							num = 0;
							break;
						}
					}
					if(i != j)
						num = (4-count[i]) * (4-count[j]);
					if(num == 0)
						continue;
					five_cards[0] = public_cards[0].point;
					five_cards[1] = public_cards[1].point;
					five_cards[2] = public_cards[2].point;
					five_cards[3] = i;
					five_cards[4] = j;
					int other_rank_info[6];
					get_rank_value(five_cards,other_rank_info,0);
					int comp_rst = comp_rank(my_rank_info,other_rank_info);
					if(comp_rst < 0)
						higher_than_me_num += num;
					higher_than_me_num= higher_than_me_num/2;
				}
			}
			if(my_rank_info[0] < 5)
				higher_than_me_num += 45; 
			prob = pow(1.0 - (float)higher_than_me_num / 1081.0,num_in_game);
		}
	}
	
	return prob;
}

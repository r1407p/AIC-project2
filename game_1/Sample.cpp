
#include "STcpClient.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <unordered_map>
#include "function.cpp"
#include <thread>

#include <chrono>
#include <ctime>
using namespace std;

#define _WIN32_WINNT 0x0600

auto time_threshold = chrono::milliseconds(1500);
double c = 1.414;


class MCTS_Node {
public:
    MCTS_Node* parent;
    std::vector<MCTS_Node*> children;
    vector<int> step;
    int playerID;
    float value;
    int visit;
    float policy;
    MCTS_Node(MCTS_Node* parent = nullptr, vector<int> step = {}, int playerID = -1) {
        this->parent = parent;
        this->step = step;
        this->playerID = playerID;
        this->value = 0;
        this->visit = 0;
        this->policy = 0;
    }    
    MCTS_Node(const MCTS_Node& other) {
        this->parent = other.parent;
        this->children = other.children;
        this->step = other.step;
        this->playerID = other.playerID;
        this->value = other.value;
        this->visit = other.visit;
        this->policy = other.policy;
        cout << "copy\n";
        cout << other.playerID<<" "<< this->playerID <<"\n";
    }
};
class MCTS{
public:
    int playerID;
    int **mapStat;
    int **sheepStat;
    MCTS_Node *root;

    MCTS(int playerID, int **mapStat, int **sheepStat){
        this->playerID = playerID;
        this->mapStat = mapStat;
        this->sheepStat = sheepStat;
        root = new MCTS_Node(nullptr, {}, playerID);
        this->simulation();
    }
    void simulation(){
        int simulation_num = 200;
        auto start = chrono::system_clock::now();
        while( simulation_num-- && chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now()-start) < time_threshold){
            // cout << "simulation_num: "<< simulation_num <<"\n";
            int **new_mapStat = copy_map(mapStat, 12);
            int **new_sheepStat = copy_map(sheepStat, 12);
            this->select(root, new_mapStat, new_sheepStat);
        }
    }
    void select(MCTS_Node *child, int **mapStat, int **sheepStat){
        // cout << "select" << child->children.size() <<"\n";
        while (child->children.size() > 0){
            // cout << "step\n";
            child->visit += 1;
            double best_score = -100000;
            MCTS_Node *best_child = nullptr;
            for (int i = 0; i < child->children.size(); i++){
                MCTS_Node *tmp = child->children[i];
                if (tmp->visit == 0){
                    best_child = tmp;
                    break;
                }else{
                    double score = tmp->value / tmp->visit + c * sqrt(log(child->visit) / tmp->visit);
                    if (score > best_score){
                        best_score = score;
                        best_child = tmp;
                    }
                }
            }
            if (best_child != nullptr){
                child = best_child;
                apply_action(mapStat, sheepStat, child->step, playerID, 12);
            }
        }
        child->visit += 1;
        if (is_terminal(mapStat, sheepStat, 12)){
            this->update(child, mapStat, sheepStat);
            return;
        }
        // cout << root->playerID <<"\n";
        // cout << child->playerID <<"\n";
        // cout << "get_actions" << child->playerID %4 + 1 <<"\n";
        vector<vector<int>> actions = get_actions(child->playerID , mapStat, sheepStat, 12);
        // cout << "actions size: "<< actions.size() <<"\n";
        for (auto action : actions){
            MCTS_Node *new_child = new MCTS_Node(child, action, child->playerID %4 + 1);
            child->children.push_back(new_child);
        }
        this->evaluate(child, mapStat, sheepStat);
    }
    void evaluate(MCTS_Node * child, int **mapStat, int **sheepStat){
        // cout << "evaluate\n";
        if (child->children.size() > 0){
            int random_child_index = rand() % child->children.size();
            child = child->children[random_child_index];
            apply_action(mapStat, sheepStat, child->step, child->playerID, 12);
        }
        int player = child->playerID;
        while(!is_terminal(mapStat, sheepStat, 12)){
            player = player % 4 + 1;
            // for(int i = 0 ;i < 12 ;i++){
            //     for(int j = 0; j < 12; j++){
            //         cout << mapStat[i][j] <<" ";
            //     }cout << "\n";
            // }
            // for(int i = 0 ;i < 12 ;i++){
            //     for(int j = 0; j < 12; j++){
            //         cout << sheepStat[i][j] <<" ";
            //     }cout << "\n";
            // }
            vector<vector<int>> actions = get_actions(player, mapStat, sheepStat, 12);
            // cout << actions.size()<<endl;
            if (actions.size() == 0){
                continue;
            }
            int random_action_index = rand() % actions.size();
            vector<int> action = actions[random_action_index];
            // cout << "apply_action\n";
            // for(auto a : action){
            //     cout << a <<" ";
            // }cout << "\n";
            apply_action(mapStat, sheepStat, action, playerID, 12);
        }
        this->update(child, mapStat, sheepStat);
    }
    void update(MCTS_Node *child, int ** mapStat, int **sheepStat){
        while(child->parent != nullptr){
            child->policy = child->visit / this->root->visit;
            child->value += cal_score(child->playerID, mapStat, sheepStat, 12);
            child = child->parent;
        }
    }
    vector<int> get_step(){
        map<vector<int>, double> mp;
        for( int i = 0;i < this->root->children.size(); i++){
            if (this->root->children[i]->visit > 0){
                auto step = this->root->children[i]->step;
                mp[step] += this->root->children[i]->value / this->root->children[i]->visit; 
            }
        }
        vector<int> step = max_element(mp.begin(), mp.end(), [](const pair<vector<int>, double> &a, const pair<vector<int>, double> &b){
            return a.second < b.second;
        })->first;
        return step;
    }
};
/*
    選擇起始位置
    選擇範圍僅限場地邊緣(至少一個方向為牆)
    
    return: init_pos
    init_pos=<x,y>,代表你要選擇的起始位置
    
*/
std::vector<int> InitPos(int mapStat[12][12], int playerID) {
    int ** convert_map_state = convertMapStat(mapStat);
    int ** sheepStat = new int*[12];
    for(int i = 0; i < 12; i++){
        sheepStat[i] = new int[12];
        for(int j = 0; j < 12; j++){
            sheepStat[i][j] = 0;
        }
    }
    auto mcts = MCTS(playerID, convert_map_state, sheepStat);
    return mcts.get_step();
    
}
/*
	產出指令
    
    input: 
	playerID: 你在此局遊戲中的角色(1~4)
    mapStat : 棋盤狀態, 為 12*12矩陣, 
					0=可移動區域, -1=障礙, 1~4為玩家1~4佔領區域
    sheepStat : 羊群分布狀態, 範圍在0~16, 為 12*12矩陣

    return Step
    Step : <x,y,m,dir> 
            x, y 表示要進行動作的座標 
            m = 要切割成第二群的羊群數量
            dir = 移動方向(1~9),對應方向如下圖所示
            1 2 3
			4 X 6
			7 8 9
*/

std::vector<int> GetStep(int playerID, int mapStat[12][12], int sheepStat[12][12]) {
    std::vector<int> step(4, 0);
    auto mcts = MCTS(playerID, convertMapStat(mapStat), convertMapStat(sheepStat));
    step = mcts.get_step();
    for (int i = 0; i < 12; i++){
        for (int j = 0; j < 12; j++){
            cout << mapStat[i][j] <<" ";
        }cout << "\n";
    }
    for (int i = 0; i < 12; i++){
        for (int j = 0; j < 12; j++){
            cout << sheepStat[i][j] <<" ";
        }cout << "\n";
    }
    return step;
}

int main()
{
	int id_package;
	int playerID;
    int mapStat[12][12];
    int sheepStat[12][12];

	// player initial
	GetMap(id_package,playerID,mapStat);
	std::vector<int> init_pos = InitPos(mapStat, playerID);
	SendInitPos(id_package,init_pos);

	while (true)
	{
		if (GetBoard(id_package, mapStat, sheepStat))
			break;

		std::vector<int> step = GetStep(playerID,mapStat,sheepStat);
		SendStep(id_package, step);
	}
}


#include "STcpClient.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <unordered_map>
#include "function.cpp"
#include <pthread.h>
#include <iomanip>
#include <chrono>
#include <ctime>
using namespace std;

#define _WIN32_WINNT 0x0600

auto time_threshold = chrono::milliseconds(2200);
double c = 1.414;
const int map_size = 12;
const int thread_num = 4;


class MCTS_Node {
public:
    MCTS_Node* parent;
    std::vector<MCTS_Node*> children;
    vector<int> step;
    int player_idx;
    float value;
    int visit;
    float policy;
    MCTS_Node(MCTS_Node* parent = nullptr, vector<int> step = {}, int player_idx = -1) {
        this->parent = parent;
        this->step = step;
        this->player_idx = player_idx;
        this->value = 0;
        this->visit = 0;
        this->policy = 0;
    }    
    // MCTS_Node(const MCTS_Node& other) {
    //     this->parent = other.parent;
    //     this->children = other.children;
    //     this->step = other.step;
    //     this->playerID = other.playerID;
    //     this->value = other.value;
    //     this->visit = other.visit;
    //     this->policy = other.policy;
    //     cout << "copy\n";
    //     cout << other.playerID<<" "<< this->playerID <<"\n";
    // }
};
class MCTS{
public:
    int ori_playerID;
    int **ori_mapStat;
    int **ori_sheepStat;
    vector<MCTS_Node *>roots;
    vector<int> players;

    MCTS(int playerID, int **mapStat, int **sheepStat, vector<int> players){
        this->ori_playerID = playerID;
        this->ori_mapStat = mapStat;
        this->ori_sheepStat = sheepStat;
        this->players = players;
        // for (int i = 0; i < thread_num; i++){
        //     roots.push_back(new MCTS_Node());
        // }
        this->start();
    }
    static DWORD WINAPI ThreadFunction(LPVOID lpParam) {
        MCTS* mcts = static_cast<MCTS*>(lpParam);
        mcts->simulation();
        return 0;
    }
    void start(){
        HANDLE threads[thread_num];
        for(int i = 0; i < thread_num; i++){
            int threadID = i +1;
            threads[i] = CreateThread(NULL, 0, ThreadFunction, this, 0, NULL);
            if (threads[i] == NULL) {
            std::cerr << "Error creating thread " << threadID << std::endl;
            return;
            }
        }
        WaitForMultipleObjects(thread_num, threads, TRUE, INFINITE);
        for(int i = 0; i < thread_num; i++){
            CloseHandle(threads[i]);
        }
    }
    void simulation(){
        auto root = new MCTS_Node();
        this->roots.push_back(root);
        int simulation_num = 40000;
        auto start = chrono::system_clock::now();
        while( simulation_num-- && chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now()-start) < time_threshold){
            // cout << "simulation_num: "<< simulation_num <<"\n";
            int **new_mapStat = copy_map(ori_mapStat, map_size);
            int **new_sheepStat = copy_map(ori_sheepStat, map_size);
            this->select(root, root, new_mapStat, new_sheepStat);
            delete_map(new_mapStat, map_size);
            delete_map(new_sheepStat, map_size);
        }
        string s = "simulation_num: " + to_string(40000- simulation_num) + "\n"; 
        cout << s ;
    }
    void select(MCTS_Node* root, MCTS_Node *current_node, int **mapStat, int **sheepStat){
        // cout << "select" << child->children.size() <<"\n";
        while (current_node->children.size() > 0){
            // cout << "step\n";
            current_node->visit += 1;
            double best_score = -10000000;
            MCTS_Node *best_child = nullptr;
            
            for (int i = 0; i < current_node->children.size(); i++){
                MCTS_Node *tmp = current_node->children[i];
                double score;
                if (tmp->visit == 0){
                    score = 10000000;
                }else{
                    score = tmp->value / tmp->visit + c * sqrt(log(root->visit) / tmp->visit);
                }
                if (score > best_score){
                    best_score = score;
                    best_child = tmp;
                }
            }
            if (best_child != nullptr){
                current_node = best_child;
                apply_action(mapStat, sheepStat, current_node->step, players[current_node->player_idx], map_size);
            }
        }
        
        current_node->visit += 1;
        if (is_terminal(mapStat, sheepStat, map_size, players)){
            this->update(root, current_node, mapStat, sheepStat);
            return;
        }
        vector<vector<int>> actions = get_actions(players[(current_node->player_idx+1)%4] , mapStat, sheepStat, map_size);
        for(auto action:actions){
            MCTS_Node *new_node = new MCTS_Node(current_node, action, (current_node->player_idx+1)%4);
            current_node->children.push_back(new_node);
        }
        this->evaluate(root, current_node, mapStat, sheepStat);
        
    }
    void evaluate(MCTS_Node* root, MCTS_Node * current_node, int **mapStat, int **sheepStat){
        // cout << "evaluate\n";
        if (current_node->children.size() > 0){
            int random_child_index = rand() % current_node->children.size();
            current_node = current_node->children[random_child_index];
            apply_action(mapStat, sheepStat, current_node->step, players[current_node->player_idx], map_size);
        }
        int player_idx = current_node->player_idx;
        while(!is_terminal(mapStat, sheepStat, map_size, players)){
            // player = player % 4 + 1;
            player_idx = (player_idx + 1) % 4;
            vector<vector<int>> actions = get_actions(players[player_idx], mapStat, sheepStat, map_size);
            if (actions.size() == 0){
                continue;
            }
            int random_action_index = rand() % actions.size();
            vector<int> action = actions[random_action_index];
            apply_action(mapStat, sheepStat, action, players[player_idx], map_size);
        }
        this->update(root, current_node, mapStat, sheepStat);
    }
    void update(MCTS_Node* root, MCTS_Node *child, int ** mapStat, int **sheepStat){
        while(child->parent != nullptr){
            child->policy = child->visit / root->visit;
            child->value += cal_score(players[child->player_idx], players[(child->player_idx+2)%4], mapStat, sheepStat, map_size);
            child = child->parent;
        }
    }
    vector<int> get_step(){
        map<vector<int>, double> mp;
        for (int i = 0; i < thread_num; i++){
            auto root = roots[i];
            for( int i = 0;i < root->children.size(); i++){
                if (root->children[i]->visit > 0){
                    auto step = root->children[i]->step;
                    mp[step] += root->children[i]->value / root->children[i]->visit; 
                }
            }
        }
        vector<int> step;
        double max_score = -100000000;
        for (auto it = mp.begin(); it != mp.end(); it++){
            if (it->second > max_score){
                max_score = it->second;
                step = it->first;
            }
        }
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
    int ** sheepStat = new int*[map_size];
    for(int i = 0; i < map_size; i++){
        sheepStat[i] = new int[map_size];
        for(int j = 0; j < map_size; j++){
            sheepStat[i][j] = 0;
        }
    }
    vector<int> players;
    for(int i = 0 ; i < 4; i++){
        int current_player = (playerID+i)%4;
        if (current_player == 0){
            current_player = 4;
        }
        players.push_back(current_player);
    }
    auto mcts = MCTS(playerID, convert_map_state, sheepStat, players);
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
    vector<int> players;
    for(int i = 0 ; i < 4; i++){
        int current_player = (playerID+i)%4;
        if (current_player == 0){
            current_player = 4;
        }
        players.push_back(current_player);
    }
    auto mcts = MCTS(playerID, convertMapStat(mapStat), convertMapStat(sheepStat), players);
    step = mcts.get_step();
    for (int i = 0; i < map_size; i++){
        for (int j = 0; j < map_size; j++){
            cout<<setw(2) << mapStat[i][j] <<" ";
        }cout << "\n";
    }
    for (int i = 0; i < map_size; i++){
        for (int j = 0; j < map_size; j++){
            cout<<setw(2) << sheepStat[i][j] <<" ";
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

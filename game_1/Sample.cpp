
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
#include <iomanip>
#include <chrono>
#include <ctime>
using namespace std;

#define _WIN32_WINNT 0x0600

auto time_threshold = chrono::milliseconds(2650);
double c = 1.414;
int MAX_SIMULATION = 400000;
int total_simulation = 0;

class MCTS_Node {
public:
    MCTS_Node* parent;
    std::vector<MCTS_Node*> children;
    vector<int> step;
    int playerID;
    float value;
    int visit;
    float policy;
    MCTS_Node(MCTS_Node* parent = nullptr, vector<int> step = {}, int playerID = 0) {
        this->parent = parent;
        this->step = step;
        this->playerID = playerID;
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
    int num_threads;
    vector<MCTS_Node*> roots;
    struct ThreadParams {
        MCTS* mcts;
        std::chrono::time_point<std::chrono::system_clock> start;
        int t;
    };
    struct ThreadResult {
        DWORD id;
        int simulation_cnt;
    };

    MCTS(int playerID, int **mapStat, int **sheepStat, int num_threads = 4){
        this->ori_playerID = playerID;
        this->ori_mapStat = mapStat;
        this->ori_sheepStat = sheepStat;
        this->num_threads = num_threads;
        for (int i = 0; i < num_threads; i++){
            this->roots.push_back(new MCTS_Node());
        }
        this->simulation();
    }
    
    void simulation(){
        int simulation_times = 40000;
        auto start = std::chrono::system_clock::now();
        std::vector<HANDLE> threads; // Windows thread handle
        vector<ThreadParams> params;
        total_simulation = 0;
        for (int t = 0; t < num_threads; t++) {
            ThreadParams param = {this, start, t};
            params.push_back(param);
        }
        for (int t = 0; t < num_threads; t++) {
            threads.push_back(CreateThread(NULL, 0, threadFunction, &params[t], 0, NULL)); // Pass the address of thread parameters
        }
        WaitForMultipleObjects(num_threads, &threads[0], TRUE, INFINITE); // Wait for all threads to finish
        for (HANDLE thread : threads) {
            CloseHandle(thread); // Close thread handles
        }
        cout << "Total simulation: " << total_simulation << "\n";
        params.clear();
        threads.clear();
        return;
    }
    static DWORD WINAPI threadFunction(LPVOID lpParam) {
        auto* params = static_cast<ThreadParams*>(lpParam);
        auto start = params->start; // Get start time from parameters
        int t = params->t; // Get thread index from parameters
        auto thread_start = start; // Dereference the pointer to get the start time
        int simulation_cnt = MAX_SIMULATION; // Example simulation times
        auto root = params->mcts->roots[t]; // Get the root node from parameters

        // Initiate different random seed
        srand(static_cast<unsigned int>(time(NULL))*t);

        while (simulation_cnt-- && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - thread_start) < time_threshold) {
            int **new_mapStat = copy_map(params->mcts->ori_mapStat, 12);
            int **new_sheepStat = copy_map(params->mcts->ori_sheepStat, 12);
            params->mcts->select(root, new_mapStat, new_sheepStat, t);
            delete_map(new_mapStat, 12);
            delete_map(new_sheepStat, 12);
        }
        // Return thread result
        ThreadResult result;
        result.id = t;
        result.simulation_cnt = simulation_cnt;
        total_simulation += MAX_SIMULATION - simulation_cnt;
        // cout << "Thread " << t << " finished with " << MAX_SIMULATION - simulation_cnt << " simulations\n";
        return reinterpret_cast<DWORD_PTR>(&result);
    }
    void select(MCTS_Node *current_node, int **mapStat, int **sheepStat, int thread_id){
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
                    score = tmp->value / tmp->visit + c * sqrt(log(roots[thread_id]->visit) / tmp->visit);
                }
                if (score > best_score){
                    best_score = score;
                    best_child = tmp;
                }
            }
            if (best_child != nullptr){
                current_node = best_child;
                apply_action(mapStat, sheepStat, current_node->step, current_node->playerID, 12);
            }
        }
        
        current_node->visit += 1;
        if (is_terminal(mapStat, sheepStat, 12)){
            this->update(current_node, mapStat, sheepStat, thread_id);
            return;
        }
        vector<vector<int>> actions = get_actions(current_node->playerID%4+1 , mapStat, sheepStat, 12);
        for(auto action:actions){
            MCTS_Node *new_node = new MCTS_Node(current_node, action, current_node->playerID%4+1);
            current_node->children.push_back(new_node);
        }
        this->evaluate(current_node, mapStat, sheepStat, thread_id);
        
    }
    void evaluate(MCTS_Node * current_node, int **mapStat, int **sheepStat, int thread_id){
        // cout << "evaluate\n";
        if (current_node->children.size() > 0){
            int random_child_index = rand() % current_node->children.size();
            current_node = current_node->children[random_child_index];
            apply_action(mapStat, sheepStat, current_node->step, current_node->playerID, 12);
        }
        int player = current_node->playerID;
        while(!is_terminal(mapStat, sheepStat, 12)){
            player = player % 4 + 1;
            vector<vector<int>> actions = get_actions(player, mapStat, sheepStat, 12);
            if (actions.size() == 0){
                continue;
            }
            int random_action_index = rand() % actions.size();
            vector<int> action = actions[random_action_index];
            apply_action(mapStat, sheepStat, action, player, 12);
        }
        this->update(current_node, mapStat, sheepStat, thread_id);
    }
    void update(MCTS_Node *child, int ** mapStat, int **sheepStat, int thread_id){
        while(child->parent != nullptr){
            child->policy = child->visit / this->roots[thread_id]->visit;
            child->value += cal_score(child->playerID, mapStat, sheepStat, 12);
            child = child->parent;
        }
    }
    vector<int> get_step(){
        map<vector<int>, double> mp;
        for (auto root : this-> roots) {
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
            cout<<setw(2) << mapStat[i][j] <<" ";
        }cout << "\n";
    }
    for (int i = 0; i < 12; i++){
        for (int j = 0; j < 12; j++){
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

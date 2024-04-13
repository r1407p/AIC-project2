
#include "STcpClient.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <unordered_map>
#include <pthread.h>
#include <iomanip>
#include <chrono>
#include <ctime>
using namespace std;

#define _WIN32_WINNT 0x0600

auto time_threshold = chrono::milliseconds(2350);
double c = 1.414;
const int map_size = 12;
const int thread_num = 4;

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <iomanip>
#include <ctime>
using namespace std;
vector<vector<int>> NEAR_{{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
vector<vector<int>> VALID_DIR({{-1, -1, 1}, {0, -1, 2}, {1, -1, 3}, {-1, 0, 4}, {1, 0, 6}, {-1, 1, 7}, {0, 1, 8}, {1, 1, 9}});
map<int, vector<int>> action_map({{1, {-1,-1}}, {2, {0,-1}}, {3, {1,-1}}, {4, {-1,0}}, {6, {1,0}}, {7, {-1,1}}, {8, {0 ,1}}, {9, {1,1}}});
vector<vector<int>> get_start_points(int **mapStat, int mapsize){
    vector<vector<int>> start_points;
    for (int i = 0; i < mapsize; i++) {
        for (int j = 0; j < mapsize; j++) {
            if (mapStat[i][j] == 0) {
                for (int k = 0; k < 4; k++) {
                    int x = i + NEAR_[k][0];
                    int y = j + NEAR_[k][1];
                    if(x<0 || x>=mapsize || y<0 || y>=mapsize){
                        start_points.push_back({i, j});
                    }else if(mapStat[x][y] == -1){
                        start_points.push_back({i, j});
                    }
                }
            }
        }
    }
    return start_points;
}

vector<vector<int>> get_actions(int playerID,int **mapStat,int **sheepStat, int mapSize) {
    int total_points = 0;
    for(int i = 0; i < mapSize; i++){
        for(int j = 0; j < mapSize; j++){
            if(mapStat[i][j] == playerID){
                total_points++;
            }
        }
    }
    if(total_points == 0){
        return get_start_points(mapStat, mapSize);
    }
    vector<vector<int>> actions;
    for(int i = 0; i < mapSize; i++){
        for(int j = 0; j < mapSize; j++){
            if(mapStat[i][j] == playerID && sheepStat[i][j] > 1){
                for(int k = 0; k < 8; k++){
                    int x = i + VALID_DIR[k][0];
                    int y = j + VALID_DIR[k][1];
                    if(x >= 0 && x < mapSize && y >= 0 && y < mapSize && mapStat[x][y] == 0){
                        for (int m = 1;m < sheepStat[i][j];m++){
                            actions.push_back({i, j, m, VALID_DIR[k][2]});
                        }
                    }
                }
            }
        }
    }
    return actions;
}

vector<int> get_players(int **mapStat,int mapsize){
    vector<int> players;
    for (int i = 0; i < mapsize; i++) {
        for (int j = 0; j < mapsize; j++) {
            if (mapStat[i][j] > 0) {
                if (find(players.begin(), players.end(), mapStat[i][j]) == players.end()) {
                    players.push_back(mapStat[i][j]);
                }
            }
        }
    }
    return players;
}

bool is_terminal(int **mapStat,int **sheepStat, int mapsize, vector<int> players){
    
    for(int i = 0; i < 4; i++){
        auto actions = get_actions(players[i], mapStat, sheepStat, mapsize);
        if (actions.size() > 0) {
            return false;
        }
    }
    return true;
}

double dfs(int playerID, int **mapStat, bool** visited, int x, int y, int mapSize){
    if(x < 0 || x >= mapSize || y < 0 || y >= mapSize || visited[y][x] || mapStat[y][x] != playerID){
        return (int)0;
    }
    visited[y][x] = true;
    double res = 1;
    for(auto dir : NEAR_){
        res += dfs(playerID, mapStat, visited, x + dir[0], y + dir[1], mapSize);
    }
    return res;
}

double cal_score(int playerID, int **mapStat, int **sheepStat, int mapSize){
    // init
    bool **visited = new bool*[mapSize];
    for(int i = 0; i < mapSize; i++){
        visited[i] = new bool[mapSize];
        for(int j = 0; j < mapSize; j++){
            visited[i][j] = false;
        }
    }

    double res = 0;
    // score
    for(int i = 0; i < mapSize; i++){
        for(int j = 0; j < mapSize; j++){
            if(mapStat[i][j] == playerID && !visited[i][j]){
                double area = dfs(playerID, mapStat, visited, i, j, mapSize);
                res += pow(area, 1.25);
            }
        }
    }
    // penalty
    for (int i = 0; i < mapSize; i++) {
        for(int j = 0; j < mapSize; j++){
            if(mapStat[i][j] == playerID && sheepStat[i][j] > 1){
                int live = 0;
                for (int k =0; k <8 ; k++){
                    int x = i + VALID_DIR[k][0];
                    int y = j + VALID_DIR[k][1];
                    if(0 <= x && x < mapSize && 0 <= y && y < mapSize && mapStat[x][y] == 0){
                        live += 1;
                    }
                }
                res -= pow(sheepStat[i][j], 1.25) * pow(0.5, live);
            }
        }
    }
    return res;
}
void init_map(int **mapStat, int **sheepStat, vector<int> init_pos, int playerID, int sheepNum){
    mapStat[init_pos[0]][init_pos[1]] = playerID;
    sheepStat[init_pos[0]][init_pos[1]] = sheepNum;
}
void apply_action(int **mapStat, int **sheepStat, vector<int> action, int playerID, int mapSize){
    if (action.size() ==2){
        init_map(mapStat, sheepStat, action, playerID, 16);
        return;
    }
    int x = action[0];
    int y = action[1];
    int m = action[2];
    int dir = action[3];
    sheepStat[x][y] -= m;
    int dx = action_map[dir][0];
    int dy = action_map[dir][1];
    while(0 <= x+dx && x+dx < mapSize && 0 <= y+dy && y+dy < mapSize && mapStat[x+dx][y+dy] == 0 ){
        x += dx;
        y += dy;
    }
    mapStat[x][y] = playerID;
    sheepStat[x][y] = m;
    // for(auto a: action ){
    //     cout << a << " ";
    // }cout  << endl;
    // cout << playerID <<endl;
    // for(int i = 0; i < mapSize; i++){
    //     for(int j = 0; j < mapSize; j++){
    //         cout << setw(2) << mapStat[i][j] <<" ";
    //     }cout << "\n";
    // }
    // for(int i = 0; i < mapSize; i++){
    //     for(int j = 0; j < mapSize; j++){
    //         cout << setw(2) << sheepStat[i][j] <<" ";
    //     }cout << "\n";
    // }
}
int **copy_map(int **map, int mapSize){
    int **new_map = new int*[mapSize];
    for(int i = 0; i < mapSize; i++){
        new_map[i] = new int[mapSize];
        for(int j = 0; j < mapSize; j++){
            new_map[i][j] = map[i][j];
        }
    }
    return new_map;
}
int** convertMapStat(int mapStat[12][12]) {
        int** convertedMapStat = new int*[12];
        for (int i = 0; i < 12; i++) {
            convertedMapStat[i] = new int[12];
            for (int j = 0; j < 12; j++) {
                convertedMapStat[i][j] = mapStat[i][j];
            }
        }
        return convertedMapStat;
    }
void delete_map(int **map, int mapSize){
    for(int i = 0; i < mapSize; i++){
        delete[] map[i];
    }
    delete[] map;
}

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
        int simulation_num = 1000000;
        auto start = chrono::system_clock::now();
        while( simulation_num-- && chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now()-start) < time_threshold){
            // cout << "simulation_num: "<< simulation_num <<"\n";
            int **new_mapStat = copy_map(ori_mapStat, map_size);
            int **new_sheepStat = copy_map(ori_sheepStat, map_size);
            this->select(root, root, new_mapStat, new_sheepStat);
            delete_map(new_mapStat, map_size);
            delete_map(new_sheepStat, map_size);
        }
        string s = "simulation_num: " + to_string(1000000- simulation_num) + "\n"; 
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
            child->value += cal_score(players[child->player_idx], mapStat, sheepStat, map_size);
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

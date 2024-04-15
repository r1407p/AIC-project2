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
                        actions.push_back({i, j, sheepStat[i][j]/2, VALID_DIR[k][2]});
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
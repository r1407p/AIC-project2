
#include "STcpClient.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <unordered_map>
/*
    選擇起始位置
    選擇範圍僅限場地邊緣(至少一個方向為牆)
    
    return: init_pos
    init_pos=<x,y>,代表你要選擇的起始位置
    
*/
#include <vector>
#include <iostream>

class Movement {
public:
    Movement(int playerID, int i, int j, int m, std::vector<std::vector<int>>& mapStat, std::vector<std::vector<int>>& sheepStat)
        : playerID(playerID), mapStat(mapStat), sheepStat(sheepStat), i(i), j(j), m(m),
          target_i(0), target_j(0), dir(0), target_m(0), res_i(0), res_j(0), res_dir(0), res_m(0) {}

    void setNext(int target_i, int target_j, int dir) {
        this->target_i = target_i;
        this->target_j = target_j;
        this->dir = dir;
    }

    void setRes(int res_i, int res_j, int res_dir, int res_m) {
        this->res_i = res_i;
        this->res_j = res_j;
        this->res_dir = res_dir;
        this->res_m = res_m;
    }

    std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> getNextState() {
        std::vector<std::vector<int>> newMapStat = mapStat;
        std::vector<std::vector<int>> newSheepStat = sheepStat;
        newMapStat[target_i][target_j] = playerID;
        newSheepStat[target_i][target_j] = target_m;
        newSheepStat[i][j] = m;
        return std::make_pair(newMapStat, newSheepStat);
    }

    std::string toString() {
        return "Current Position: (" + std::to_string(i) + ", " + std::to_string(j) + "), Remaining Sheep: " + std::to_string(m)
               + ", Target Position: (" + std::to_string(target_i) + ", " + std::to_string(target_j) + "), Direction: " + std::to_string(dir);
    }

public:
    int playerID;
    std::vector<std::vector<int>> mapStat;
    std::vector<std::vector<int>> sheepStat;
    int i, j, m;
    int target_i, target_j, dir, target_m;
    int res_i, res_j, res_dir, res_m;
};

std::vector<Movement> getPossibleInitPos(std::vector<std::vector<int>>& mapStat, int playerID) {
    int h = mapStat.size();
    int w = mapStat[0].size();
    std::vector<std::pair<int, int>> possible_pos;
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            if (mapStat[i][j] == 0) {
                if (i == 0 || i == h - 1 || j == 0 || j == w - 1) {
                    possible_pos.push_back(std::make_pair(i, j));
                } else if (mapStat[i - 1][j] == -1 || mapStat[i + 1][j] == -1 || mapStat[i][j - 1] == -1 || mapStat[i][j + 1] == -1) {
                    possible_pos.push_back(std::make_pair(i, j));
                }
            }
        }
    }
    std::vector<Movement> pos;
    for (const auto& p : possible_pos) {
        std::vector<std::vector<int>> next_map = mapStat;
        next_map[p.first][p.second] = playerID;
        std::vector<std::vector<int>> next_sheep = mapStat;
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                if (next_sheep[i][j] == -1) {
                    next_sheep[i][j] = 0;
                } else if (next_sheep[i][j] > 0) {
                    next_sheep[i][j] = 16;
                }
            }
        }
        next_sheep[p.first][p.second] = 16;
        pos.emplace_back(playerID, p.first, p.second, 16, next_map, next_sheep);
    }
    return pos;
}
std::vector<int> separate(int m) {
    switch (m) {
        case 2: return {1};
        case 3: return {1, 2};
        case 4: return {1, 2, 3};
        case 5: return {2, 3};
        case 6: return {2, 3, 4};
        case 7: return {3, 4};
        case 8: return {4, 2, 6};
        case 9: return {4, 2, 7};
        case 10: return {5, 3, 7};
        case 11: return {6, 4, 8};
        case 12: return {6, 4, 8};
        case 13: return {6, 4, 9};
        case 14: return {7, 3, 10};
        case 15: return {7, 3, 11};
        case 16: return {8, 4, 12};
        default: return {};
    }
}
std::vector<std::vector<int>> valid_dir = {{-1, -1, 1}, {-1, 0, 2}, {-1, 1, 3}, {0, -1, 4}, {0, 1, 6}, {1, -1, 7}, {1, 0, 8}, {1, 1, 9}};

std::vector<int> getTarget(int i, int j, int l, int r, const std::vector<std::vector<int>>& mapStat) {
    int target_i = i, target_j = j;
    while (true) {
        target_i += l;
        target_j += r;
        // valid target
        if (0 <= target_i && target_i < 12 && 0 <= target_j && target_j < 12 && mapStat[target_i][target_j] == 0) {
            continue;
        }
        // invalid target
        else {
            target_i -= l;
            target_j -= r;
            break;
        }
    }
    if (target_i == i && target_j == j) {
        return {-1, -1}; // None in Python
    }
    return {target_i, target_j};
}
std::vector<Movement> getPossibleMove(int playerID, std::vector<std::vector<int>>& mapStat, std::vector<std::vector<int>>& sheepStat) {
    std::vector<Movement> current_pos;
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            if (mapStat[i][j] == playerID && sheepStat[i][j] > 1) {
                current_pos.push_back(Movement(playerID, i, j, static_cast<int>(sheepStat[i][j]), mapStat, sheepStat));
            }
        }
    }
    
    std::vector<Movement> move;
    for (auto& pos : current_pos) {
        for (const auto& v : valid_dir) {
            int l = v[0];
            int r = v[1];
            int dir = v[2];
            int target_i, target_j;
            auto targets = getTarget(pos.i, pos.j, l, r, mapStat);
            target_i = targets[0];
            target_j = targets[1];
            if (target_i == -1 || target_j == -1) {
                continue;
            }
            Movement new_pos = pos;
            new_pos.setNext(target_i, target_j, dir);
            move.push_back(new_pos);
        }
    }
    
    std::vector<Movement> all_possible_move;
    for (auto& pos : move) {
        for (int remain : separate(pos.m)) {
            Movement new_pos = pos;
            new_pos.m -= remain;
            new_pos.target_m = remain;
            all_possible_move.push_back(new_pos);
        }
    }
    
    return all_possible_move;
}
int calculateScore(const std::vector<std::vector<int>>& mapStat, const std::vector<std::vector<int>>& sheepStat, int playerID) {
    // Initialize a hash table to store the size of each connected region for the player
    std::map<std::pair<int, int>, bool> connectedRegions;

    // Iterate through the mapStat to find connected regions of the player's cells
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            if (mapStat[i][j] == playerID) {
                // If the cell is already visited, continue to the next cell
                if (connectedRegions.find({i, j}) != connectedRegions.end()) {
                    continue;
                }

                // Perform a depth-first search to find the connected region size
                std::vector<std::pair<int, int>> stack = {{i, j}};
                int regionSize = 0;
                while (!stack.empty()) {
                    auto v = stack.back();
                    int x = v.first;
                    int y = v.second;
                    stack.pop_back();
                    // If the cell is already visited, continue to the next cell
                    if (connectedRegions.find({x, y}) != connectedRegions.end()) {
                        continue;
                    }
                    // Mark the cell as visited
                    connectedRegions[{x, y}] = true;
                    regionSize++;
                    // Check the neighboring cells
                    auto directions = std::vector<std::pair<int, int>>{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}; 
                    for (auto dir : directions) {
                        int dx = dir.first;
                        int dy = dir.second;
                        int nx = x + dx;
                        int ny = y + dy;
                        // Check if the neighboring cell is within the boundaries and belongs to the player
                        if (0 <= nx && nx < 12 && 0 <= ny && ny < 12 && mapStat[nx][ny] == playerID) {
                            stack.push_back({nx, ny});
                        }
                    }
                }
                // Store the size of the connected region
                connectedRegions[{i, j}] = regionSize;
            }
        }
    }

    // Calculate the score based on the size of each connected region raised to the power of 1.25
    double score = 0.0;
    for (const auto& key: connectedRegions) {
        int regionSize = key.second;
        score += std::pow(regionSize, 2.5);
    }

    // Round the score to the nearest integer
    int roundedScore = static_cast<int>(std::round(score));
    return roundedScore;
}
std::vector<int> InitPos(int mapStat[12][12]) {
    std::pair<int, int> pos = std::make_pair(0, 0);
    std::vector<std::vector<int>> map_(12, std::vector<int>(12, 0));
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            map_[i][j] = mapStat[i][j];
        }
    }
    // get current movable position
    std::vector<std::vector<Movement>> tree;
    tree.push_back(getPossibleInitPos(map_, 100));
    for (auto& pos : tree[0]) {
        pos.setRes(pos.i, pos.j, pos.dir, pos.m);
    }
    int total = tree[0].size();
    std::cout << total << std::endl;
    int current_layer = 0;
    while (total < 1000) {
        current_layer++;
        tree.push_back(std::vector<Movement>());
        for (auto& pos : tree[current_layer - 1]) {
            auto states = pos.getNextState();
            auto current_mapStat = states.first;
            auto current_sheepStat = states.second;
            auto new_moves = getPossibleMove(100, current_mapStat, current_sheepStat);
            for (auto& new_pos : new_moves) {
                new_pos.setRes(pos.res_i, pos.res_j, pos.res_dir, pos.res_m);
                tree[current_layer].push_back(new_pos);
            }
        }
        if (tree[current_layer].empty()) {
            current_layer--;
            break;
        }
        std::cout << total << std::endl;
        total += tree[current_layer].size();
    }
    
    // calculate score
    std::vector<std::pair<int, int>> score;
    std::map<std::pair<int, int>, int> dict;
    for (auto& pos : tree[current_layer]) {
        auto stats = pos.getNextState();
        auto current_mapStat = stats.first;
        auto current_sheepStat = stats.second;
        score.push_back(std::make_pair(calculateScore(current_mapStat, current_sheepStat, 100), pos.res_i * 100 + pos.res_j));
        dict[{pos.res_i, pos.res_j}]++;
    }
    int max_cnt = 0;
    std::vector<int> chosen_pos = {tree[0][0].i, tree[0][0].j};
    for (auto& key : dict) {
        int value = key.second;
        if (value > max_cnt) {
            max_cnt = value;
            chosen_pos = {key.first.first,key.first.second}; 
        }
    }

    return chosen_pos;
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
#include <vector>
#include <tuple>
#include <algorithm>

std::tuple<int, int> get_target(int i, int j, int l, int r, int mapStat[12][12]) {
    int target_i = i, target_j = j;
    while (true) {
        target_i += l;
        target_j += r;
        // valid target
        if (0 <= target_i && target_i < 12 && 0 <= target_j && target_j < 12 && mapStat[target_i][target_j] == 0) {
            continue;
        }
        // invalid target
        else {
            target_i -= l;
            target_j -= r;
            break;
        }
    }
    if (target_i == i && target_j == j) {
        return std::make_tuple(-1, -1);
    }
    return std::make_tuple(target_i, target_j);
}

std::tuple<int, int, int, int> calculate_score(int target_i, int target_j, int mapStat[12][12], int playerID) {
    int score = 0;
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            if (mapStat[i][j] == playerID) {
                score += std::abs(i - target_i) + std::abs(j - target_j);
            }
        }
    }
    return std::make_tuple(score, target_i, target_j, playerID);
}

int calculate_sheep(int i, int j, int m, int target_i, int target_j, int mapStat[12][12], int sheepStat[12][12]) {
    int middle = std::abs(i + target_i) + std::abs(j + target_j) + 1;
    m -= middle;
    return m;
}

std::vector<int> GetStep(int playerID, int mapStat[12][12], int sheepStat[12][12]) {
    std::vector<int> step(4, 0);

    std::vector<std::tuple<int, int, int>> current_pos;
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            if (mapStat[i][j] == playerID && sheepStat[i][j] > 1) {
                current_pos.push_back(std::make_tuple(i, j, sheepStat[i][j]));
            }
        }
    }

    std::vector<std::tuple<int, int, int, int, int, int>> move;
    for (auto a : current_pos) {
		auto i = std::get<0>(a);
		auto j = std::get<1>(a);
		auto m = std::get<2>(a);

        std::vector<std::tuple<int, int, int>> valid_dir = {{-1, -1, 1}, {-1, 0, 2}, {-1, 1, 3}, {0, -1, 4}, {0, 1, 6}, {1, -1, 7}, {1, 0, 8}, {1, 1, 9}};
        for(auto n : valid_dir) {
			auto l = std::get<0>(n);
			auto r = std::get<1>(n);
			auto dir = std::get<2>(n);

            auto target = get_target(i, j, l, r, mapStat);
			auto target_i = std::get<0>(target);
			auto target_j = std::get<1>(target);

            if (target_i == -1 && target_j == -1) {
                continue;
            }
            move.push_back(std::make_tuple(i, j, m, target_i, target_j, dir));
        }
    }

    std::vector<std::tuple<int, int, int, int, int, int, int>> score_move;
    for (auto single_move: move) {
		auto i = std::get<0>(single_move);
		auto j = std::get<1>(single_move);
		auto m = std::get<2>(single_move);
		auto target_i = std::get<3>(single_move);
		auto target_j = std::get<4>(single_move);
		auto dir = std::get<5>(single_move);

        auto score_ = calculate_score(target_i, target_j, mapStat, playerID);
		auto score = std::get<0>(score_);
		target_i = std::get<1>(score_);
		target_j = std::get<2>(score_);
		auto playerID = std::get<3>(score_);
        score_move.push_back(std::make_tuple(score, i, j, m, target_i, target_j, dir));
    }

    std::sort(score_move.begin(), score_move.end());

    auto score = score_move[0];
	int i = std::get<1>(score);
	int j = std::get<2>(score);
	int m = std::get<3>(score);
	int target_i = std::get<4>(score);
	int target_j = std::get<5>(score);
	int dir = std::get<6>(score);

    int subsheep = calculate_sheep(i, j, m, target_i, target_j, mapStat, sheepStat);

    step[0] = i;
    step[1] = j;
    step[2] = subsheep;
    step[3] = dir;

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
	std::vector<int> init_pos = InitPos(mapStat);
	SendInitPos(id_package,init_pos);

	while (true)
	{
		if (GetBoard(id_package, mapStat, sheepStat))
			break;

		std::vector<int> step = GetStep(playerID,mapStat,sheepStat);
		SendStep(id_package, step);
	}
}


#include "STcpClient.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>
/*
    選擇起始位置
    選擇範圍僅限場地邊緣(至少一個方向為牆)
    
    return: init_pos
    init_pos=<x,y>,代表你要選擇的起始位置
    
*/
std::vector<int> InitPos(int mapStat[12][12]) {
    std::vector<int> init_pos(2, 0);

    int metadata[12][12][4] = {0};
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            if (mapStat[i][j] == 0) {
                for (int k = i; k < 12; ++k) {
                    for (int l = j; l < 12; ++l) {
                        metadata[k][l][0] += 1;
                    }
                }

                for (int k = i; k < 12; ++k) {
                    for (int l = j; l >= 0; --l) {
                        metadata[k][l][1] += 1;
                    }
                }

                for (int k = i; k >= 0; --k) {
                    for (int l = j; l < 12; ++l) {
                        metadata[k][l][2] += 1;
                    }
                }

                for (int k = i; k >= 0; --k) {
                    for (int l = j; l >= 0; --l) {
                        metadata[k][l][3] += 1;
                    }
                }
            }
        }
    }

    int distance[12][12];
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            distance[i][j] = 10000;
        }
    }

    std::vector<std::vector<int>> priority;
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            for (int k = 0; k < 4; ++k) {
                distance[i][j] = std::min(distance[i][j], abs(metadata[i][j][k] - 20));
            }
            priority.push_back({distance[i][j], i, j});
        }
    }

    sort(priority.begin(), priority.end());
	

    for (auto& a : priority) {
		int i = a[1];
		int j = a[2];
        if (mapStat[i][j] == -1) {
            continue;
        }
        for (auto a : std::vector<std::vector<int>>{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}) {
			int l = a[0];
			int r = a[1];
            if (0 <= i + l && i + l < 12 && 0 <= j + r && j + r < 12 && mapStat[i + l][j + r] == -1) {
                init_pos[0] = i;
                init_pos[1] = j;
                return init_pos;
            }
        }
    }

    // if no viable position, return the first empty position
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            if (mapStat[i][j] == 0) {
                init_pos[0] = i;
                init_pos[1] = j;
                return init_pos;
            }
        }
    }

    return init_pos;
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

import STcpClient
import numpy as np
import random
from pprint import pprint

'''
    選擇起始位置
    選擇範圍僅限場地邊緣(至少一個方向為牆)
    
    return: init_pos
    init_pos=[x,y],代表起始位置
    
'''


def InitPos(mapStat):
    init_pos = [0, 0]
    metadata = np.zeros((12, 12, 4))
    for i in range(12):
        for j in range(12):
            if mapStat[i][j] == 0:
                
                for k in range(i, 12):
                    for l in range(j, 12):
                        metadata[k][l][0] += 1

                for k in range(i, 12):
                    for l in range(j, -1, -1):
                        metadata[k][l][1] += 1
                
                for k in range(i, -1, -1):
                    for l in range(j, 12):
                        metadata[k][l][2] += 1
                
                for k in range(i, -1, -1):
                    for l in range(j, -1, -1):
                        metadata[k][l][3] += 1
    distance = np.full((12, 12), 10000)
    priority = []
    for i in range(12):
        for j in range(12):
            for k in range(4):
                distance[i][j] = min(distance[i][j], abs(metadata[i][j][k]-22)) # hack 20 is viable for 16
            priority.append((distance[i][j], i, j))
    priority.sort()
    for _, i, j in priority:
        if mapStat[i][j] == -1:
            continue
        for (l,r) in (-1, 0), (1, 0), (0, -1), (0, 1):
            if 0 <= i+l < 12 and 0 <= j+r < 12 and mapStat[i+l][j+r] == -1:
                return [i, j]
    # if no viable position, return the first empty position
    for i in range(12):
        for j in range(12):
            if mapStat[i][j] == 0:
                return [i, j]
'''
    產出指令
    
    input: 
    playerID: 你在此局遊戲中的角色(1~4)
    mapStat : 棋盤狀態(list of list), 為 12*12矩陣, 
              0=可移動區域, -1=障礙, 1~4為玩家1~4佔領區域
    sheepStat : 羊群分布狀態, 範圍在0~16, 為 12*12矩陣

    return Step
    Step : 3 elements, [(x,y), m, dir]
            x, y 表示要進行動作的座標 
            m = 要切割成第二群的羊群數量
            dir = 移動方向(1~9),對應方向如下圖所示
            1 2 3
            4 X 6
            7 8 9
'''
def get_target(i, j, l, r, mapStat):
    target_i, target_j = i, j
    while 1:
        target_i += l
        target_j += r
        # valid target
        if 0 <= target_i < 12 and 0 <= target_j < 12 and mapStat[target_i][target_j] == 0:
            continue
        # invalid target
        else:
            target_i -= l
            target_j -= r
            break
    if target_i == i and target_j == j:
        return None, None
    return target_i, target_j
    
valid_dir = [(-1,-1,1), (0,-1,2), (1,-1,3), (-1,0,4), (1,0,6), (-1,1,7), (0,1,8), (1,1,9)]
# valid_dir = [(-1,-1,1), (-1,0,2), (-1,1,3), (0,-1,4), (0,1,6), (1,-1,7), (1,0,8), (1,1,9)]

def calculate_score(target_i, target_j, mapStat):
    score = 0
    for i in range(12):
        for j in range(12):
            if mapStat[i][j] == playerID:
                score += abs(i-target_i) + abs(j-target_j)
    return -score
def calculate_sheep(i, j, m, target_i, target_j, mapStat, sheepStat):
    middle = abs(i-target_i)+abs(j-target_j) +1
    return int(max(1, m-middle))
    
    

def GetStep(playerID, mapStat, sheepStat):
    step = [(0, 0), 0, 1]
    current_pos = []
    for i in range(12):
        for j in range(12):
            if mapStat[i][j] == playerID and sheepStat[i][j] > 1:
                current_pos.append((i, j, sheepStat[i][j]))
    print(current_pos)
    move = []
    for i, j, m in current_pos:
        for l, r, dir in valid_dir:
            target_i, target_j = get_target(i, j, l, r, mapStat)
            if target_i is None:
                continue
            move.append((i, j, m, target_i, target_j, dir))
    print(move)
    score_move = []
    for i, j, m, target_i, target_j, dir in move:
        score = calculate_score(target_i, target_j, mapStat)
        score_move.append((score, i, j, m, target_i, target_j, dir))
    score_move.sort()
    _, i, j, m, target_i, target_j, dir = score_move[0]
    subsheep = calculate_sheep(i, j, m, target_i, target_j, mapStat, sheepStat)
    return [(i, j), subsheep, dir]
    '''
    Write your code here
    
    '''
    return step


# player initial
(id_package, playerID, mapStat) = STcpClient.GetMap()
init_pos = InitPos(mapStat)
STcpClient.SendInitPos(id_package, init_pos)

# start game
while (True):
    (end_program, id_package, mapStat, sheepStat) = STcpClient.GetBoard()
    if end_program:
        STcpClient._StopConnect()
        break
    Step = GetStep(playerID, mapStat, sheepStat)

    STcpClient.SendStep(id_package, Step)

if __name__ == '__main__':
    import random
    mapStat = [[ random.randint(0,1) for i in range(12)] for j in range(12)]
    InitPos(mapStat)
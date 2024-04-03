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
    '''
        Write your code here

    '''
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
                distance[i][j] = min(distance[i][j], abs(metadata[i][j][k]-20)) # hack 20 is viable for 16
            priority.append((distance[i][j], i, j))
    priority.sort()
    for _, i, j in priority:
        if mapStat[i][j] == -1:
            continue
        for (l,r) in (-1, 0), (1, 0), (0, -1), (0, 1):
            if 0 <= i+l < 12 and 0 <= j+r < 12 and mapStat[i+l][j+r] == -1:
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
def GetStep(playerID, mapStat, sheepStat):
    step = [(0, 0), 0, 1]
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

# if __name__ == '__main__':
#     import random
#     mapStat = [[ random.randint(0,1) for i in range(12)] for j in range(12)]
#     InitPos(mapStat)
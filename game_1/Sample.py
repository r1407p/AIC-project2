import STcpClient
import numpy as np
import random
from copy import deepcopy
from pprint import pprint

'''
    選擇起始位置
    選擇範圍僅限場地邊緣(至少一個方向為牆)
    
    return: init_pos
    init_pos=[x,y],代表起始位置
    
'''
def is_valid(x, y, mapStat):
    if mapStat[x][y] != 0:
        return False
    for (l,r) in (-1, 0), (1, 0), (0, -1), (0, 1):
        if  mapStat[x+l][y+r] == -1 or x+l < 0 or x+l >= 12 or y+r < 0 or y+r >= 12:
            return True


def generate(x, y):
    res = []
    for i in range(x, y+1):
        res.append((i, x))
        res.append((i, y))
        res.append((x, i))
        res.append((y, i))
    return res
    
def InitPos(mapStat):
    init_pos = [0, 0]
    center = 5  # Center index of a 12x12 board
    for i in range(center, -1, -1):  # Iterate from the middle ring towards the outer rings
        positions = generate(i,11 -i)
        print(positions)
        for pos in positions:
            print(pos, mapStat[pos[0]][pos[1]])
            if is_valid(pos[0], pos[1], mapStat):
                init_pos = pos
                return init_pos
    return init_pos  # If no valid starting position is found, return the default [0, 0]
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
class Movement:
    def __init__(self, playerID, i,j,m, mapStat, sheepStat):
        self.playerID = playerID
        self.mapStat = mapStat
        self.sheepStat = sheepStat
        
        self.i = i
        self.j = j
        self.m = m
        
        self.target_i = 0
        self.target_j = 0
        self.dir = 0
        self.target_m = 0
        
        self.res_i = 0
        self.res_j = 0
        self.res_dir = 0
        self.res_m = 0
    
    def set_next(self, target_i, target_j, dir):
        self.target_i = target_i
        self.target_j = target_j
        self.dir = dir
    
    def set_res(self, res_i, res_j, res_dir, res_m):
        self.res_i = res_i
        self.res_j = res_j
        self.res_dir = res_dir
        self.res_m = res_m
    
    def get_next_state(self):
        new_mapStat = deepcopy(self.mapStat)
        new_sheepStat = deepcopy(self.sheepStat)
        new_mapStat[self.target_i][self.target_j] = playerID  
        new_sheepStat[self.target_i][self.target_j] = self.target_m
        new_sheepStat[self.i][self.j] = self.m
        return new_mapStat, new_sheepStat
    
    def __str__(self) -> str:
        return f"Current Position: ({self.i}, {self.j}), Remaining Sheep: {self.m}, Target Position: ({self.target_i}, {self.target_j}), Direction: {self.dir}"
    def __repr__(self) -> str:
        return f"Current Position: ({self.i}, {self.j}), Remaining Sheep: {self.m}, Target Position: ({self.target_i}, {self.target_j}), Direction: {self.dir}"

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

def get_possible_move(playerID, mapStat, sheepStat):    
    current_pos = []
    for i in range(12):
        for j in range(12):
            if mapStat[i][j] == playerID and sheepStat[i][j] > 1:
                current_pos.append(Movement(playerID, i, j, int(sheepStat[i][j]), mapStat, sheepStat))
                
    # get all possible move without number of sheep
    move = []
    for pos in current_pos:
        for l, r, dir in valid_dir:
            target_i, target_j = get_target(pos.i, pos.j, l, r, mapStat)
            if target_i is None:
                continue
            new_pos = deepcopy(pos)
            new_pos.set_next(target_i, target_j, dir)
            move.append(new_pos)
    # get all possible move with number of sheep
    all_possible_move = []
    for pos in move:
        new_pos = deepcopy(pos)
        remain = pos.m //2
        new_pos.m -= remain 
        new_pos.target_m = remain
        all_possible_move.append(new_pos)
    return all_possible_move

def calculate_score(mapStat, sheepStat, playerID):
    # Initialize a dictionary to store the size of each connected region for the player
    connected_regions = {}

    # Iterate through the mapStat to find connected regions of the player's cells
    for i in range(12):
        for j in range(12):
            if mapStat[i][j] == playerID:
                # If the cell is already visited, continue to the next cell
                if (i, j) in connected_regions:
                    continue
                
                # Perform a depth-first search to find the connected region size
                stack = [(i, j)]
                region_size = 0
                while stack:
                    x, y = stack.pop()
                    # If the cell is already visited, continue to the next cell
                    if (x, y) in connected_regions:
                        continue
                    # Mark the cell as visited
                    connected_regions[(x, y)] = True
                    region_size += 1
                    # Check the neighboring cells
                    for dx, dy in [(1, 0), (-1, 0), (0, 1), (0, -1)]:
                        nx, ny = x + dx, y + dy
                        # Check if the neighboring cell is within the boundaries and belongs to the player
                        if 0 <= nx < 12 and 0 <= ny < 12 and mapStat[nx][ny] == playerID:
                            stack.append((nx, ny))
                # Store the size of the connected region
                connected_regions[(i, j)] = region_size
    
    # Calculate the score based on the size of each connected region raised to the power of 1.25
    score = sum(region_size ** 2.5 for region_size in connected_regions.values())
    
    # Round the score to the nearest integer
    rounded_score = score
    
    return rounded_score

def GetStep(playerID, mapStat, sheepStat):
    step = [(0, 0), 0, 1]
    # get current movable position
    tree = []
    tree.append(get_possible_move(playerID, mapStat, sheepStat)) # generate one layer
    for pos in tree[0]:
        pos.set_res(pos.i, pos.j,pos.dir, pos.m)
    total = len(tree[0])
    current_layer = 0
    while total < 1000:
        current_layer += 1
        tree.append([])
        for pos in tree[current_layer-1]:
            current_mapStat, current_sheepStat = pos.get_next_state()
            new_moves = get_possible_move(playerID, current_mapStat, current_sheepStat)
            for new_pos in new_moves:
                new_pos.set_res(pos.res_i, pos.res_j, pos.res_dir, pos.res_m)
                tree[current_layer].append(new_pos)
                
        if len(tree[current_layer]) == 0:
            current_layer -= 1
            break    
        
        total += len(tree[current_layer])
    
    # calculate score
    score = []
    for pos in tree[current_layer]:
        score.append((calculate_score(current_mapStat, current_sheepStat, playerID), pos.res_i, pos.res_j, pos.res_dir, pos.res_m))
    score.sort(reverse=True)
    # pprint(score)
    # return the best move
    step[0] = (score[0][1], score[0][2])
    step[1] = score[0][4]
    step[2] = score[0][3]
    # pprint(step)
    return step
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


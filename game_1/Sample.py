import STcpClient
import numpy as np
import random
from copy import deepcopy
from pprint import pprint
import time
'''
    選擇起始位置
    選擇範圍僅限場地邊緣(至少一個方向為牆)
    
    return: init_pos
    init_pos=[x,y],代表起始位置
    
'''
def get_possible_init_pos(mapStat):
    h, w = mapStat.shape
    possible_pos = []
    for i in range(h):
        for j in range(w):
            if mapStat[i][j] == 0:
                if i == 0 or i == h-1 or j == 0 or j == w-1:
                    possible_pos.append((i, j))
                elif mapStat[i-1][j] == -1 or mapStat[i+1][j] == -1 or mapStat[i][j-1] == -1 or mapStat[i][j+1] == -1:
                    possible_pos.append((i, j))
    pos = []
    for p in possible_pos:
        next_map = deepcopy(mapStat)
        next_map[p[0]][p[1]] = playerID
        next_sheep = deepcopy(mapStat)
        for i in range(h):
            for j in range(w):
                if next_sheep[i][j] == -1:
                    next_sheep[i][j] = 0
                elif next_sheep[i][j] > 0:
                    next_sheep[i][j] = 16
        next_sheep[p[0]][p[1]] = 16
        pos.append(Movement(playerID, p[0], p[1], 16, next_map, next_sheep))
    return pos

def InitPos(mapStat):
    pos = (0, 0)
    # get current movable position
    tree = []
    tree.append(get_possible_init_pos(mapStat))
    for pos in tree[0]:
        pos.set_res(pos.i, pos.j,pos.dir, pos.m)
    total = len(tree[0])
    print(total)
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
        print(total)
        total += len(tree[current_layer])
    
    # calculate score
    score = []
    dict = {}
    for pos in tree[current_layer]:
        current_mapStat, current_sheepStat = pos.get_next_state()
        score.append((calculate_score(current_mapStat, current_sheepStat, playerID), pos.res_i, pos.res_j, pos.res_dir, pos.res_m))
        if (pos.res_i, pos.res_j) not in dict:
            dict[(pos.res_i, pos.res_j)] = 1
        else:
            dict[(pos.res_i, pos.res_j)] += 1
    max_cnt = 0
    choosen_pos = (tree[0][0].i, tree[0][0].j)
    for key in dict:
        if dict[key] > max_cnt:
            max_cnt = dict[key]
            choosen_pos = key

    return choosen_pos
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
        assert False
    return target_i, target_j
    
valid_dir = [(-1,-1,1), (0,-1,2), (1,-1,3), (-1,0,4), (1,0,6), (-1,1,7), (0,1,8), (1,1,9)]    
def deparate(m):
    m_ = [
        [],
        [],
        [1], #2
        [1, 2], #3
        [2, 3], #4
        [3, 2], #5
        [3, 2, 4], #6
        [3, 4], #7
        [4, 2, 6], #8
        [4, 2, 7], #9
        [5, 3, 7], #10
        [6, 4, 8], #11
        [6, 4,  8], #12
        [6, 4, 9], #13
        [7, 3, 10], #14
        [7, 3, 11], #15
        [8, 4,  12] #16
    ]
    if 2<= m <= 16:
        return m_[m]
    else:
        return []
class Action:
    def __init__(self, i, j, m, target_i, target_j, dir, new_m):
        self.ori_i = i  
        self.ori_j = j
        self.ori_m = m
        self.target_i = target_i
        self.target_j = target_j
        self.dir = dir
        self.new_m = new_m
    def __str__(self) -> str:
        return f"Current Position: ({self.ori_i}, {self.ori_j}), Remaining Sheep: {self.ori_m - self.new_m}, Target Position: ({self.next_i}, {self.next_j}), Direction: {self.dir}, New Sheep: {self.new_m}"
    
def get_possible_action(playerID, mapStat, sheepStat):
    movable_pos = []
    for i in range(12):
        for j in range(12):
            if mapStat[i][j] == playerID and sheepStat[i][j] > 1:
                movable_pos.append((i, j, int(sheepStat[i][j])))
    movable_pos_dir = []
    for i, j, m in movable_pos:
        for l, r, dir in valid_dir:
            next_i, next_j = i+l, j+r
            if 0 <= next_i < 12 and 0 <= next_j < 12 and mapStat[next_i][next_j] == 0:
                movable_pos_dir.append((i, j, m, l, r, dir))
    all_possible_action = []
    for i, j, m, l, r, dir in movable_pos_dir:
        target_i, target_j = get_target(i, j, l, r, mapStat)
        for remain in deparate(m):
            all_possible_action.append(Action(i, j, m-remain, target_i, target_j, dir, remain))
    
    return all_possible_action
    
def get_possible_move(playerID, mapStat, sheepStat):
    all_possible_move = []
    actions = get_possible_action(playerID, mapStat, sheepStat)
    for action in actions:
        
        new_mapStat = deepcopy(mapStat)
        new_sheepStat = deepcopy(sheepStat)
        new_move = Movement(playerID, action.ori_i, action.ori_j, action.ori_m, new_mapStat, new_sheepStat)
        new_move.set_next(action.target_i, action.target_j, action.dir)
        new_move.target_m = action.new_m
        all_possible_move.append(new_move)
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
    score = sum(region_size ** 1.25 for region_size in connected_regions.values())
    for i  in range(12):
        for j in range(12):
            live = 0
            if mapStat[i][j] == playerID and sheepStat[i][j] > 1:
                for l in range(-1, 2, 1):
                    for r in range(-1, 2, 1):
                        if l == 0 and r == 0:
                            continue
                        next_i = i + l
                        next_j = j + r
                        if 0 <= next_i < 12 and 0 <= next_j < 12 and mapStat[next_i][next_j] == 0:
                            live += 1
                score -= sheepStat[i][j]**1.25 * (0.5 ** live)
    # Round the score to the nearest integer
    rounded_score = score
    
    return rounded_score

def GetStep(playerID, mapStat, sheepStat):
    start_time = time.time()
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
            new_moves = random.sample(new_moves, int(len(new_moves) * 0.8))
            if len(new_moves) == 0:
                tree[current_layer].append(pos)
                continue
            # new_moves = random.sample(new_moves, int(len(new_moves) * 0.7))
            for new_pos in new_moves:
                new_pos.set_res(pos.res_i, pos.res_j, pos.res_dir, pos.res_m)
                tree[current_layer].append(new_pos)
                
        if len(tree[current_layer]) == 0:
            current_layer -= 1
            break    
        
        total += len(tree[current_layer])
    print(current_layer)
    # calculate score
    score = []
    for pos in tree[current_layer]:
        current_mapStat, current_sheepStat = pos.get_next_state()
        score.append((calculate_score(current_mapStat, current_sheepStat, playerID), pos.res_i, pos.res_j, pos.res_dir, pos.res_m))
    score.sort(reverse=True)
    # if len(score) < 20:
    #     for i in range(3):
    #         print(tree[i])
    #     for _ in score:
    #         print(_)
    # return the best move
    step[0] = (score[0][1], score[0][2])
    step[1] = score[0][4]
    step[2] = score[0][3]
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


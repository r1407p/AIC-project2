import STcpClient
import random
import math
from numpy import zeros, array
import time
import copy

'''
    選擇起始位置
    選擇範圍僅限場地邊緣(至少一個方向為牆)
    
    return: init_pos
    init_pos=[x,y],代表起始位置
    
'''

def generate(x, y):
    res = []
    for i in range(x, y+1):
        res.append((i, x))
        res.append((i, y))
        res.append((x, i))
        res.append((y, i))
    return res

def is_valid(x, y, mapStat):
    if mapStat[x][y] != 0:
        return False
    for (l,r) in (-1, 0), (1, 0), (0, -1), (0, 1):
        if  mapStat[x+l][y+r] == -1 or x+l < 0 or x+l >= 12 or y+r < 0 or y+r >= 12:
            return True

def InitPos(mapStat):
    init_pos = [0, 0]
    center = 5  # Center index of a 12x12 board
    for i in range(center, -1, -1):  # Iterate from the middle ring towards the outer rings
        positions = generate(i,11 -i)
        # print(positions)
        for pos in positions:
            # print(pos, mapStat[pos[0]][pos[1]])
            if is_valid(pos[0], pos[1], mapStat):
                init_pos = pos
                return init_pos
    return init_pos  # If no valid starting position is found, return the default [0, 0]
    # if no viable position, return the first empty position

class Node:
    def __init__(self, state, parent=None):
        self.state = state
        self.parent = parent
        self.children = []
        self.visits = 0
        self.score = 0

    def is_fully_expanded(self):
        return len(self.children) == len(self.state.get_legal_actions())

    def is_terminal(self):
        return self.state.is_terminal()

    def select_child(self, exploration_factor=1.4):
        log_total_visits = math.log(self.visits)
        best_child = None
        best_ucb_score = float("-inf")

        for child in self.children:
            exploitation = child.score / child.visits
            exploration = exploration_factor * math.sqrt(log_total_visits / child.visits)
            ucb_score = exploitation + exploration
            if ucb_score > best_ucb_score:
                best_ucb_score = ucb_score
                best_child = child
        return best_child


    def expand(self):
        print("expand")
        legal_actions = self.state.get_legal_actions()
        action = random.choice(legal_actions)
        # next_state = copy.deepcopy(self.state)
        next_state = self.state.take_action(action)
        next_state.switch_player()
        next_state.print_board()
        child = Node(next_state, self)
        self.children.append(child)
        # print("node player: ", self.state.current_player)
        
        return child

    def backpropagate(self, score):
        # print("backpropagate {}".format(self.visits))
        self.visits += 1
        self.score += score
        if self.parent:
            self.parent.backpropagate(score)

class MCTS:
    def __init__(self, state, timeout=3):
        self.root = Node(state)
        self.timeout = timeout
        self.max_iterations = 3
        self.level = 3
        self.state = state

    def search(self):
        print("search")
        it = 0
        while(it<self.max_iterations):
            # print("current_player: ", self.state.current_player)
            node = self.select_node_to_expand()
            # self.state.print_board()
            if node.is_terminal():
                # print("terminate")
                score = self.simulate(node)
            else:
                # print("not terminate")
                node = node.expand()
                score = self.simulate(node)
            node.backpropagate(score)
            it+=1
        print("done search")
        return self.root.select_child(exploration_factor=0)

    def select_node_to_expand(self):
        node = self.root
        print("root: ", self.root.state.current_player)
        while not node.is_terminal() and node.is_fully_expanded():
            node = node.select_child()
        return node
    
    def simulate(self, node):
        state = node.state
        it = 0
        while not state.is_terminal() and it <= self.level:
            it+=1
            print("simulate")
            if(len(state.get_legal_actions())==0):
                state.switch_player()
                continue
            action = random.choice(state.get_legal_actions())
            state = state.take_action(action)
            state.print_board()
            state.switch_player()
            
        return state.get_score()
    
class BattleSheep:
    def __init__(self, id_package, board, current_player):
        self.state_id = 1
        self.id_package = id_package
        self.board = board
        self.current_player = current_player
        self.init_pos = InitPos(self.board)
        STcpClient.SendInitPos(self.id_package, self.init_pos)
        self.board_size = 12
        self.num_players = 4
        self.num_sheep = 16 
        self.sheeps_distribute = zeros([12, 12])
        # self.sheeps_distribute[]
        self.players_positions = [[] for _ in range(self.num_players)]
        self.players_positions[0].append(self.init_pos)
    
    def print_board(self):
        # file = "log.txt"
        # with open(file, "a") as f:
        #     for i in range(self.board_size):
        #         for j in range(self.board_size):
        #             if(self.board[i, j]==-1):
        #                 f.write("  ")
        #             elif(self.board[i, j]==0):
        #                 f.write(". ")
        #             else:
        #                 f.write(str(int(self.board[i, j]))+" ")
        #         f.write("\n")
        #     f.write("===================")
        #     f.write("\n")
        for i in range(self.board_size):
            for j in range(self.board_size):
                if(self.board[i, j]==-1):
                    print("  ", end="")
                elif(self.board[i, j]==0):
                    print(". ", end="")
                else:
                    print(str(int(self.board[i, j]))+" ", end="")
            print("")
        print("===================")

    def get_legal_actions(self):
        dirs = [(-1,-1), (0,-1), (1,-1), (-1,0), (1,0), (-1,1), (0,1), (1,1)]   
        player_positions = self.players_positions[self.current_player-1]
        legal_actions = []
        for pos in player_positions:
            x, y = pos
            # Check all possible moves
            for dx, dy in dirs:
                b = False
                nx, ny = x, y
                while True:
                    nx, ny = nx + dx, ny + dy
                    if 0 <= nx < self.board_size and 0 <= ny < self.board_size and self.board[nx][ny] == 0:
                        b = True
                        # Continue moving in the same direction
                        continue
                    else:
                        nx, ny = nx - dx, ny - dy
                        break
            # If we've moved at all in this direction, add it as a legal action
                if (nx, ny) != (x, y) and b:
                    legal_actions.append((x, y, nx, ny))
        # print("player {} legal_actions: {}".format(self.current_player, len(legal_actions)))
        # for la in legal_actions:
        #     print("{},{}->{},{} ".format(la[0], la[1], la[2], la[3]), end="")
        # print("")
        return legal_actions
            # todo sheep state

    def switch_player(self):
        self.current_player = (self.current_player) % self.num_players+1

    def take_action(self, action):
        # print("current_player: ", self.current_player)
        # new_state = BattleSheep(self.id_package, self.board, self.current_player)
        new_state = copy.deepcopy(self)
        x, y, nx, ny = action
        new_state.board[nx][ny] = self.current_player  # 执行动作，更新新的棋盘副本s
        new_state.players_positions = self.players_positions # 创建一个新的玩家位置列表的副本，避免直接修改原始列表
        new_state.players_positions[self.current_player - 1].append((nx, ny))  # 更新新的玩家位置列表
        print("player {} take_action: {},{}->{},{}".format(self.current_player, x, y, nx, ny))
        # self.print_board()
        new_state.state_id+=1
        return new_state

    def check_can_move(self, player):
        dirs = [(-1,-1), (0,-1), (1,-1), (-1,0), (1,0), (-1,1), (0,1), (1,1)]  
        player_positions = self.players_positions[player]
        for pos in player_positions:
            x, y = pos
            # Check all possible moves: up, down, left, right
            for dx, dy in dirs:
                nx, ny = x, y
                while True:
                    nx, ny = nx + dx, ny + dy
                    if 0 <= nx < self.board_size and 0 <= ny < self.board_size and self.board[nx][ny] == 0:
                        continue
                    else:
                        break
                if (nx, ny) != (x, y):
                    return True
        return False

    def is_terminal(self):
        for player in range(self.num_players):
            if(self.check_can_move(player)):
                return False
        return True
    
    def calculate_score(self, mapStat, sheepStat, playerID):
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

    def get_score(self):
        return self.calculate_score(self.board, self.sheeps_distribute, self.current_player)
        # todo evaluate 

    def GetStep(self, action):
        dirs = [(-1,-1), (0,-1), (1,-1), (-1,0), (1,0), (-1,1), (0,1), (1,1)]  
        step = [(0, 0), 0, 1]
        xdir = action[2]-action[0]
        ydir = action[3]-action[1]
        if xdir>0:
            xdir=1
        elif xdir<0:
            xdir=-1
        if ydir>0:
            ydir=1
        elif ydir<0:
            ydir=-1
        step[0] = (action[0], action[1])
        step[1] = dirs.index((xdir, ydir))
        if(step[1]<=3):
            step[1]+=1
        else:
            step[1]+=2
        step[2] = 1
        return step
    
    def update_player(self):
        for i in range(self.board_size):
            for j in range(self.board_size):
                if(1<=self.board[i, j]<=4):
                    if((i, j) not in self.players_positions[int(self.board[i, j])-1]):
                        self.players_positions[int(self.board[i, j])-1].append((i, j))

    
    def game(self):
        it=2
        while(it) :
            it-=1
            # file = "log.txt"
            # with open(file, "a") as f:
            #     f.write("current_player: "+str(self.current_player)+"\n")
            #     for i in range(len(self.players_positions[self.current_player-1])):
            #         f.write("{} ".format(self.players_positions[self.current_player-1][i]))
            #     f.write("\n")
            mcts = MCTS(state)
            (end_program, self.id_package, self.board, self.sheeps_distribute) = STcpClient.GetBoard()
            self.update_player()
            if end_program:
                STcpClient._StopConnect()
                break
            chosen_node = mcts.search()
            best_action = chosen_node.state.get_legal_actions()[0]
            x, y, nx, ny = best_action
            # with open(file, "a") as f:
            #     f.write("player {} take_action: {},{}->{},{}\n".format(self.current_player, x, y, nx, ny))
            Step = self.GetStep(best_action)
            STcpClient.SendStep(self.id_package, Step)

(id_package, current_player , board) = STcpClient.GetMap()
state = BattleSheep(id_package, board, current_player)
state.game()

    

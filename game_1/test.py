import subprocess
import pyautogui
import time
import subprocess
import pyautogui
import time

# Function to run the AI_game.exe and parse the scoreboard
    
def run_game():
    print("Running game...")
    process = subprocess.Popen([".\AI_game.exe"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print("Game started")
    while True:
        windows = pyautogui.getWindowsWithTitle("battle sheep")
        if len(windows) > 0:
            break
        time.sleep(1)
    print("Game window found")
    for window in windows:
        window.close()
    print("Game window closed")
    # lines = []
    # output, _ = process.communicate()  # 获取终止子进程后的输出
    # lines = output.splitlines()  # 将输出按行分割成列表
    lines = []
    process.stdout.flush()
    output = process.stdout.read()
    lines = output.decode().splitlines()
    print("Game ended")
    print(lines)
    print("Game ended")
    process.kill()  # 结束子进程
    
    print("Game ended")
    scoreboard_lines = []
    for line in lines:
        if line.startswith('Score Board'):
            scoreboard_lines = lines[lines.index(line) + 1:]
            break
    return scoreboard_lines

# Run the game 100 times
scoreboards = []
try:
    for i in range(100):
        scoreboard = run_game()
        scoreboards.append(scoreboard)
except KeyboardInterrupt as e:
    print(e)
    print("Game ended unexpectedly")
    print("Scoreboards:")
    
    for scoreboard in scoreboards:
        print("=====================================")    
        for line in scoreboard:
            print(line)
    print("Exiting...")
    exit()
        
        
        
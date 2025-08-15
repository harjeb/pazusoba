#!/usr/bin/env python3
"""
模拟珠子移动的可视化 - 显示移动完成后的棋盘状态（消除前）
"""

import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np

def create_movement_simulation():
    """创建珠子移动模拟可视化"""
    
    # 真实程序输出数据
    test_case = {
        'board': 'LBGLLDHHGRGBBRHBHRRRGRBBDRDDRR',
        'normal': {
            'score': 95, 'combo': 6, 'steps': 19,
            'route': 'RDRDLLUULDRURURDLUR',
            'start_pos': 9
        },
        'diagonal': {
            'score': 116, 'combo': 7, 'steps': 12,
            'route': 'LDL LD  DD L',  # 实际8步移动
            'start_pos': 10
        }
    }
    
    # 创建 2x4 的子图布局
    fig, axes = plt.subplots(2, 4, figsize=(24, 12))
    
    # 第一行：4方向模式
    draw_board(axes[0, 0], test_case['board'], "Initial Board\n4-Direction Mode")
    draw_movement_route(axes[0, 1], test_case['board'], test_case['normal'], "4-Direction Route\n(First 10 moves)", 'blue')
    final_board_normal = simulate_orb_movement(test_case['board'], test_case['normal'])
    draw_board(axes[0, 2], final_board_normal, "After Movement\n4-Direction\n(Before combo elimination)")
    draw_results(axes[0, 3], test_case['normal'], "4-Direction Results", 'lightblue')
    
    # 第二行：8方向斜对角模式
    draw_board(axes[1, 0], test_case['board'], "Initial Board\n8-Direction Diagonal")
    draw_diagonal_movement_route(axes[1, 1], test_case['board'], test_case['diagonal'], "8-Direction Route\n(All 8 moves)", 'red')
    final_board_diagonal = simulate_diagonal_movement(test_case['board'], test_case['diagonal'])
    draw_board(axes[1, 2], final_board_diagonal, "After Movement\n8-Direction Diagonal\n(Before combo elimination)")
    draw_results(axes[1, 3], test_case['diagonal'], "8-Direction Results", 'lightcoral')
    
    plt.tight_layout()
    plt.suptitle(f'Orb Movement Simulation\nBoard: {test_case["board"]}', 
                 fontsize=16, fontweight='bold', y=0.98)
    
    # 打印分析
    print("=== Orb Movement Simulation ===")
    print(f"Initial Board: {test_case['board']}")
    print()
    print("4-Direction Mode:")
    print(f"  Route: {test_case['normal']['route'][:20]}... ({len(test_case['normal']['route'])} moves)")
    print(f"  After movement: {final_board_normal}")
    print(f"  Program result: Score {test_case['normal']['score']}, Combo {test_case['normal']['combo']}")
    print()
    print("8-Direction Diagonal Mode:")
    moves = parse_diagonal_route(test_case['diagonal']['route'])
    print(f"  Route: '{test_case['diagonal']['route']}' ({len(moves)} moves)")
    print(f"  After movement: {final_board_diagonal}")
    print(f"  Program result: Score {test_case['diagonal']['score']}, Combo {test_case['diagonal']['combo']}")
    
    return fig

def parse_diagonal_route(route):
    """解析斜向路线"""
    moves = []
    i = 0
    
    while i < len(route):
        char = route[i]
        if char == ' ':
            i += 1
            continue
        elif char in 'UDLR':
            is_diagonal = i > 0 and route[i-1] == ' '
            moves.append((char, is_diagonal))
            i += 1
        else:
            i += 1
    
    return moves

def simulate_orb_movement(board_string, result):
    """模拟普通移动的珠子移动"""
    rows, cols = 5, 6
    # 将棋盘字符串转换为2D数组
    board = []
    for i in range(rows):
        row = []
        for j in range(cols):
            idx = i * cols + j
            if idx < len(board_string):
                row.append(board_string[idx])
            else:
                row.append(' ')
        board.append(row)
    
    start_pos = result['start_pos']
    route = result['route']
    
    # 起始位置
    start_row = start_pos // cols
    start_col = start_pos % cols
    
    # 记录被拖动的珠子
    dragged_orb = board[start_row][start_col]
    
    # 模拟拖动过程（只显示前10步）
    curr_row, curr_col = start_row, start_col
    
    direction_map = {
        'U': (-1, 0), 'D': (1, 0), 'L': (0, -1), 'R': (0, 1)
    }
    
    moves_to_simulate = min(10, len(route))
    for move_char in route[:moves_to_simulate]:
        if move_char in direction_map:
            dr, dc = direction_map[move_char]
            new_row = max(0, min(rows-1, curr_row + dr))
            new_col = max(0, min(cols-1, curr_col + dc))
            
            # 交换珠子位置
            board[curr_row][curr_col], board[new_row][new_col] = board[new_row][new_col], board[curr_row][curr_col]
            
            curr_row, curr_col = new_row, new_col
    
    # 转换回字符串
    result_string = ''
    for i in range(rows):
        for j in range(cols):
            result_string += board[i][j]
    
    return result_string

def simulate_diagonal_movement(board_string, result):
    """模拟斜向移动的珠子移动"""
    rows, cols = 5, 6
    # 将棋盘字符串转换为2D数组
    board = []
    for i in range(rows):
        row = []
        for j in range(cols):
            idx = i * cols + j
            if idx < len(board_string):
                row.append(board_string[idx])
            else:
                row.append(' ')
        board.append(row)
    
    start_pos = result['start_pos']
    moves = parse_diagonal_route(result['route'])
    
    # 起始位置
    start_row = start_pos // cols
    start_col = start_pos % cols
    
    # 记录被拖动的珠子
    dragged_orb = board[start_row][start_col]
    
    # 模拟拖动过程
    curr_row, curr_col = start_row, start_col
    
    direction_map = {
        'U': (-1, 0), 'D': (1, 0), 'L': (0, -1), 'R': (0, 1)
    }
    
    # 斜向移动的近似方向
    diagonal_directions = {
        'U': [(-1, -1), (-1, 1)],  # 上左、上右
        'D': [(1, -1), (1, 1)],    # 下左、下右
        'L': [(-1, -1), (1, -1)],  # 上左、下左
        'R': [(-1, 1), (1, 1)]     # 上右、下右
    }
    
    diagonal_index = 0
    for move_char, is_diagonal in moves:
        if move_char in direction_map:
            if is_diagonal:
                # 斜向移动
                if move_char in diagonal_directions:
                    dr, dc = diagonal_directions[move_char][diagonal_index % 2]
                    diagonal_index += 1
                else:
                    dr, dc = direction_map[move_char]
            else:
                # 普通移动
                dr, dc = direction_map[move_char]
            
            new_row = max(0, min(rows-1, curr_row + dr))
            new_col = max(0, min(cols-1, curr_col + dc))
            
            # 交换珠子位置
            board[curr_row][curr_col], board[new_row][new_col] = board[new_row][new_col], board[curr_row][curr_col]
            
            curr_row, curr_col = new_row, new_col
    
    # 转换回字符串
    result_string = ''
    for i in range(rows):
        for j in range(cols):
            result_string += board[i][j]
    
    return result_string

def draw_board(ax, board_string, title):
    """绘制棋盘"""
    rows, cols = 5, 6
    orb_colors = {
        'R': '#FF4444', 'B': '#4444FF', 'G': '#44FF44',
        'L': '#FFFF44', 'D': '#8844FF', 'H': '#FF44FF'
    }
    
    # 绘制网格
    for i in range(rows + 1):
        ax.axhline(y=i, color='black', linewidth=1)
    for j in range(cols + 1):
        ax.axvline(x=j, color='black', linewidth=1)
    
    # 绘制珠子和位置编号
    for i in range(rows):
        for j in range(cols):
            orb_idx = i * cols + j
            if orb_idx < len(board_string):
                orb = board_string[orb_idx]
                color = orb_colors.get(orb, '#CCCCCC')
                
                circle = patches.Circle((j + 0.5, rows - i - 0.5), 0.35,
                                      facecolor=color, edgecolor='black', linewidth=2)
                ax.add_patch(circle)
                ax.text(j + 0.5, rows - i - 0.5, orb,
                       ha='center', va='center', fontsize=12, fontweight='bold', color='white')
                
                # 位置编号（小号）
                ax.text(j + 0.8, rows - i - 0.8, str(orb_idx),
                       ha='center', va='center', fontsize=7,
                       bbox=dict(boxstyle="round,pad=0.05", facecolor="white", alpha=0.8))
    
    ax.set_xlim(0, cols)
    ax.set_ylim(0, rows)
    ax.set_aspect('equal')
    ax.set_title(title, fontsize=12, fontweight='bold')
    ax.set_xticks([])
    ax.set_yticks([])

def draw_movement_route(ax, board_string, result, title, path_color):
    """绘制普通移动路线"""
    draw_board(ax, board_string, title)
    
    rows, cols = 5, 6
    start_pos = result['start_pos']
    route = result['route']
    
    # 起始位置
    start_row = start_pos // cols
    start_col = start_pos % cols
    
    # 高亮起始珠子
    start_circle = patches.Circle((start_col + 0.5, rows - start_row - 0.5), 0.45,
                                facecolor='none', edgecolor='lime', linewidth=5)
    ax.add_patch(start_circle)
    
    # 计算移动路径（前10步）
    curr_row, curr_col = start_row, start_col
    positions = [(curr_col + 0.5, rows - curr_row - 0.5)]
    
    direction_map = {
        'U': (-1, 0), 'D': (1, 0), 'L': (0, -1), 'R': (0, 1)
    }
    
    moves_shown = min(10, len(route))
    for i, move_char in enumerate(route[:moves_shown]):
        if move_char in direction_map:
            dr, dc = direction_map[move_char]
            curr_row = max(0, min(rows-1, curr_row + dr))
            curr_col = max(0, min(cols-1, curr_col + dc))
            positions.append((curr_col + 0.5, rows - curr_row - 0.5))
    
    # 绘制移动路径
    colors = ['blue', 'green', 'purple', 'brown', 'pink', 'cyan', 'orange']
    for i in range(1, len(positions)):
        color = colors[(i-1) % len(colors)]
        start_coord = positions[i-1]
        end_coord = positions[i]
        ax.plot([start_coord[0], end_coord[0]], [start_coord[1], end_coord[1]], 
               color=color, linewidth=4, alpha=0.8)
        # 标记步骤
        ax.text(end_coord[0], end_coord[1], str(i),
               ha='center', va='center', fontsize=8, fontweight='bold', color='white',
               bbox=dict(boxstyle="circle,pad=0.1", facecolor=color, alpha=0.9))
    
    # 终点
    if positions:
        final_pos = positions[-1]
        ax.plot(final_pos[0], final_pos[1], 's', color='red', markersize=15,
               markeredgecolor='white', markeredgewidth=2)
    
    # 路线信息
    info_text = f"Drag route: {route[:15]}...\nShowing: {moves_shown}/{len(route)} moves"
    ax.text(0.02, 0.98, info_text, transform=ax.transAxes, fontsize=9,
           verticalalignment='top', bbox=dict(boxstyle="round,pad=0.3", facecolor="white", alpha=0.9))

def draw_diagonal_movement_route(ax, board_string, result, title, path_color):
    """绘制斜向移动路线"""
    draw_board(ax, board_string, title)
    
    rows, cols = 5, 6
    start_pos = result['start_pos']
    
    # 起始位置
    start_row = start_pos // cols
    start_col = start_pos % cols
    
    # 高亮起始珠子
    start_circle = patches.Circle((start_col + 0.5, rows - start_row - 0.5), 0.45,
                                facecolor='none', edgecolor='lime', linewidth=5)
    ax.add_patch(start_circle)
    
    # 解析移动
    moves = parse_diagonal_route(result['route'])
    
    # 计算移动路径
    curr_row, curr_col = start_row, start_col
    positions = [(curr_col + 0.5, rows - curr_row - 0.5)]
    
    direction_map = {
        'U': (-1, 0), 'D': (1, 0), 'L': (0, -1), 'R': (0, 1)
    }
    
    diagonal_directions = {
        'U': [(-1, -1), (-1, 1)],
        'D': [(1, -1), (1, 1)],
        'L': [(-1, -1), (1, -1)],
        'R': [(-1, 1), (1, 1)]
    }
    
    diagonal_index = 0
    for move_char, is_diagonal in moves:
        if move_char in direction_map:
            if is_diagonal:
                if move_char in diagonal_directions:
                    dr, dc = diagonal_directions[move_char][diagonal_index % 2]
                    diagonal_index += 1
                else:
                    dr, dc = direction_map[move_char]
            else:
                dr, dc = direction_map[move_char]
            
            curr_row = max(0, min(rows-1, curr_row + dr))
            curr_col = max(0, min(cols-1, curr_col + dc))
            positions.append((curr_col + 0.5, rows - curr_row - 0.5))
    
    # 绘制移动路径
    for i in range(1, len(positions)):
        start_coord = positions[i-1]
        end_coord = positions[i]
        move_char, is_diagonal = moves[i-1]
        
        if is_diagonal:
            # 斜向移动用橙色箭头
            ax.annotate('', xy=end_coord, xytext=start_coord,
                      arrowprops=dict(arrowstyle='->', color='orange', lw=5, alpha=0.9))
            # 标记
            mid_x = (start_coord[0] + end_coord[0]) / 2
            mid_y = (start_coord[1] + end_coord[1]) / 2
            ax.text(mid_x, mid_y, f'{i}\nDIAG', ha='center', va='center',
                   fontsize=9, fontweight='bold', color='orange',
                   bbox=dict(boxstyle="round,pad=0.2", facecolor="white", edgecolor='orange', alpha=0.9))
        else:
            # 普通移动用蓝色线段
            ax.plot([start_coord[0], end_coord[0]], [start_coord[1], end_coord[1]], 
                   color='blue', linewidth=4, alpha=0.8)
            # 标记步骤
            direction_symbols = {'U': '↑', 'D': '↓', 'L': '←', 'R': '→'}
            symbol = direction_symbols.get(move_char, move_char)
            ax.text(end_coord[0], end_coord[1], f'{i}\n{symbol}',
                   ha='center', va='center', fontsize=9, fontweight='bold', color='white',
                   bbox=dict(boxstyle="circle,pad=0.1", facecolor='blue', alpha=0.9))
    
    # 终点
    if positions:
        final_pos = positions[-1]
        ax.plot(final_pos[0], final_pos[1], 's', color='red', markersize=15, 
               markeredgecolor='white', markeredgewidth=2)
    
    # 路线信息
    diagonal_count = sum(1 for _, is_diagonal in moves if is_diagonal)
    regular_count = len(moves) - diagonal_count
    info_text = f"Drag route: '{result['route']}'\nMoves: {len(moves)} (R:{regular_count}, D:{diagonal_count})"
    ax.text(0.02, 0.98, info_text, transform=ax.transAxes, fontsize=9,
           verticalalignment='top', bbox=dict(boxstyle="round,pad=0.3", facecolor="white", alpha=0.9))

def draw_results(ax, result, title, color):
    """显示结果"""
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 10)
    
    # 标题
    ax.text(5, 9, title, ha='center', va='center', fontsize=14, fontweight='bold')
    
    # 结果
    metrics = [
        f"Score: {result['score']}",
        f"Combos: {result['combo']}",
        f"Program Steps: {result['steps']}"
    ]
    
    for i, metric in enumerate(metrics):
        ax.text(5, 7-i*1.5, metric, ha='center', va='center', fontsize=12,
               bbox=dict(boxstyle="round,pad=0.3", facecolor=color, alpha=0.7))
    
    ax.set_xticks([])
    ax.set_yticks([])
    for spine in ax.spines.values():
        spine.set_visible(False)

if __name__ == "__main__":
    try:
        import matplotlib.pyplot as plt
        import matplotlib.patches as patches
        import numpy as np
    except ImportError:
        print("Need matplotlib and numpy:")
        print("pip install matplotlib numpy")
        exit(1)
    
    print("Creating orb movement simulation...")
    
    fig = create_movement_simulation()
    plt.savefig('orb_movement_simulation.png', dpi=200, bbox_inches='tight')
    print("\nSaved: orb_movement_simulation.png")
    plt.close()
    
    print("\nOrb movement simulation complete!")
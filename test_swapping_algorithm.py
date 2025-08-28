import math

def build_swapping_optimized_path(center_x, center_y, row, column):
    """构建拥抱交换机制的优化路径"""
    path = []
    
    # 定义3x3目标区域
    min_x = max(0, center_x - 1)
    max_x = min(row - 1, center_x + 1)
    min_y = max(0, center_y - 1)
    max_y = min(column - 1, center_y + 1)
    
    print(f"Target 3x3 area: ({min_x},{min_y}) to ({max_x},{max_y})")
    
    # 策略1：蛇形路径覆盖整个棋盘
    current_x, current_y = 0, 0
    path.append((current_x, current_y))
    
    # 创建蛇形路径
    while current_x < row:
        # 向右移动
        while current_y < column:
            if current_x != 0 or current_y != 0:
                path.append((current_x, current_y))
            current_y += 1
        current_y -= 1
        
        if current_x + 1 < row:
            current_x += 1
            path.append((current_x, current_y))
            
            # 向左移动
            while current_y > 0:
                current_y -= 1
                path.append((current_x, current_y))
            
            if current_x + 1 < row:
                current_x += 1
                path.append((current_x, current_y))
        else:
            break
        
        if len(path) > 30:
            break
    
    # 策略2：漩涡效应
    vortex_path = []
    for radius in range(1, 4):
        for angle in range(0, 360, 30):
            rad = angle * math.pi / 180.0
            x = center_x + int(radius * math.cos(rad))
            y = center_y + int(radius * math.sin(rad))
            
            x = max(0, min(row - 1, x))
            y = max(0, min(column - 1, y))
            
            vortex_path.append((x, y))
    
    path.extend(vortex_path)
    
    # 策略3：目标区域内精细调整
    for pass_num in range(2):
        for i in range(min_x, max_x + 1):
            for j in range(min_y, max_y + 1):
                if (i + j + pass_num) % 2 == 0:
                    path.append((i, j))
    
    return path

def print_path(path):
    """打印路径"""
    print(f"Path with {len(path)} steps:")
    for i, (x, y) in enumerate(path[:20]):  # 只显示前20步
        print(f"({x},{y})", end=" ")
        if (i + 1) % 10 == 0:
            print()
    if len(path) > 20:
        print("\n... (showing first 20 steps)")
    print()

def main():
    print("=== Testing Swapping-Optimized Algorithm ===")
    
    # 测试用例：目标在 (2,3)，棋盘 5x6
    center_x, center_y = 2, 3
    row, column = 5, 6
    
    path = build_swapping_optimized_path(center_x, center_y, row, column)
    print_path(path)
    
    print("\n=== Algorithm Analysis ===")
    print("1. Snake pattern: Covers entire board systematically")
    print("2. Vortex pattern: Creates swirling motion around target")
    print("3. Fine-tuning: Precise movements within target area")
    print("\nKey insights:")
    print("- Each step swaps current orb with adjacent orb")
    print("- Path design embraces swapping mechanism")
    print("- Multiple passes gradually shuffle orbs into position")
    print("- Systematic coverage ensures all orbs participate in swapping")
    
    # 分析路径特点
    print(f"\n=== Path Analysis ===")
    print(f"Total steps: {len(path)}")
    print(f"Unique positions visited: {len(set(path))}")
    
    # 计算在目标区域内的步数
    target_positions = [(i, j) for i in range(max(0, center_x-1), min(row, center_x+2)) 
                       for j in range(max(0, center_y-1), min(column, center_y+2))]
    target_steps = sum(1 for pos in path if pos in target_positions)
    print(f"Steps within target area: {target_steps}")
    print(f"Coverage of target area: {target_steps / len(target_positions) * 100:.1f}%")

if __name__ == "__main__":
    main()
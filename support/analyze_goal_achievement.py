#!/usr/bin/env python3
"""
Detailed analysis of each test case to check actual goal achievement and move steps
"""

def parse_board(board_str):
    """Parse board string to list"""
    board = []
    orb_map = {'R': 1, 'B': 2, 'G': 3, 'L': 4, 'D': 5, 'H': 6}
    
    for c in board_str[:30]:
        board.append(orb_map.get(c, 0))
    
    return board

def print_board(board, title=""):
    """Print the board"""
    if title:
        print(f"\n{title}")
    
    orb_symbols = ['.', 'R', 'B', 'G', 'L', 'D', 'H']
    for i in range(5):
        row = ""
        for j in range(6):
            pos = i * 6 + j
            row += orb_symbols[board[pos]] + " "
        print(row)

def find_r_orb_positions(board):
    """Find all R orb positions"""
    positions = []
    for i in range(30):
        if board[i] == 1:  # R orb
            positions.append(i)
    return positions

def check_3x3_completion(board, top_row, top_col):
    """Check if 3x3 area is complete with R orbs"""
    for i in range(3):
        for j in range(3):
            pos = (top_row + i) * 6 + (top_col + j)
            if board[pos] != 1:  # Not R orb
                return False
    return True

def calculate_min_moves_to_3x3(board, top_row, top_col):
    """Calculate minimum moves needed to complete 3x3"""
    # Find R orbs outside the target area
    outside_orbs = []
    inside_r_count = 0
    
    for i in range(30):
        if board[i] == 1:  # R orb
            row = i // 6
            col = i % 6
            in_target = (top_row <= row < top_row + 3 and top_col <= col < top_col + 3)
            
            if in_target:
                inside_r_count += 1
            else:
                outside_orbs.append(i)
    
    # Find empty spaces in target area
    empty_positions = []
    for i in range(3):
        for j in range(3):
            pos = (top_row + i) * 6 + (top_col + j)
            if board[pos] != 1:  # Not R orb
                empty_positions.append(pos)
    
    # Calculate minimum moves (each orb needs to move to an empty space)
    needed_orbs = 9 - inside_r_count
    available_orbs = len(outside_orbs)
    
    if available_orbs < needed_orbs:
        return -1  # Impossible
    
    # Estimate minimum moves based on Manhattan distance
    total_distance = 0
    for i in range(min(needed_orbs, len(outside_orbs))):
        orb_pos = outside_orbs[i]
        empty_pos = empty_positions[i]
        
        orb_row, orb_col = orb_pos // 6, orb_pos % 6
        empty_row, empty_col = empty_pos // 6, empty_pos % 6
        
        distance = abs(orb_row - empty_row) + abs(orb_col - empty_col)
        total_distance += distance
    
    # Rough estimate: each orb needs at least its distance in moves
    # But we can move multiple orbs in sequence
    min_moves = max(needed_orbs, total_distance // 3)  # Heuristic
    
    return min_moves

def analyze_case(board_str, case_name, target_info):
    """Analyze a specific case"""
    print(f"\n{'='*60}")
    print(f"ANALYZING {case_name}")
    print(f"{'='*60}")
    
    board = parse_board(board_str)
    print_board(board, f"Initial Board:")
    
    top_row, top_col, expected_score = target_info
    
    print(f"\nTarget 3x3 Area: ({top_row}, {top_col}) to ({top_row+2}, {top_col+2})")
    
    # Count current R orbs in target area
    current_r_in_target = 0
    for i in range(3):
        for j in range(3):
            pos = (top_row + i) * 6 + (top_col + j)
            if board[pos] == 1:
                current_r_in_target += 1
    
    print(f"Current R orbs in target area: {current_r_in_target}/9")
    
    # Check if already complete
    is_complete = check_3x3_completion(board, top_row, top_col)
    print(f"3x3 already complete: {'YES' if is_complete else 'NO'}")
    
    # Find all R orbs
    r_positions = find_r_orb_positions(board)
    print(f"Total R orbs on board: {len(r_positions)}")
    print(f"R orb positions: {[f'({p//6},{p%6})' for p in r_positions]}")
    
    # Calculate minimum moves needed
    min_moves = calculate_min_moves_to_3x3(board, top_row, top_col)
    if min_moves >= 0:
        print(f"Estimated minimum moves needed: {min_moves}")
    else:
        print("IMPOSSIBLE - Not enough R orbs")
    
    # Show target area details
    print(f"\nTarget Area Details:")
    for i in range(3):
        row_str = ""
        for j in range(3):
            pos = (top_row + i) * 6 + (top_col + j)
            if board[pos] == 1:
                row_str += "[R] "
            else:
                orb_symbols = ['.', 'B', 'G', 'L', 'D', 'H']
                row_str += f" {orb_symbols[board[pos]-1]} " if board[pos] > 1 else " . "
        print(f"  {row_str}")
    
    return {
        'current_r_in_target': current_r_in_target,
        'is_complete': is_complete,
        'total_r_orbs': len(r_positions),
        'min_moves': min_moves,
        'possible': min_moves >= 0
    }

def main():
    """Main analysis function"""
    # Test cases with their target information
    test_cases = [
        ("LBGLLDHHGRGBBRHBHRRRGRBBDRDDRR", "Case 1", (2, 0, 4200)),
        ("HBRHLGBBRRDDRRHBHBHBDRDHRDDBRR", "Case 2", (0, 0, 4200)),
        ("DHLRHHRBRRDHRRBLLHHHRGRDLDDRLB", "Case 3", (1, 0, 5300)),
        ("LDBHRDRDLLHRLHDLHRHRGLRRRLLRHB", "Case 4", (1, 3, 4200)),
        ("BRHRLHRBRRGRHHHRLGDBGDBLLRDLRG", "Case 5", (0, 1, 5300)),
    ]
    
    results = []
    
    for board_str, case_name, target_info in test_cases:
        result = analyze_case(board_str, case_name, target_info)
        results.append((case_name, result))
    
    # Summary
    print(f"\n{'='*60}")
    print("SUMMARY - GOAL ACHIEVEMENT ANALYSIS")
    print(f"{'='*60}")
    
    possible_count = 0
    for case_name, result in results:
        current_r = result['current_r_in_target']
        total_r = result['total_r_orbs']
        min_moves = result['min_moves']
        possible = result['possible']
        
        if possible:
            possible_count += 1
            status = f"POSSIBLE (est. {min_moves} moves)"
        else:
            status = "IMPOSSIBLE"
        
        print(f"{case_name}: {status}")
        print(f"  Current R in target: {current_r}/9")
        print(f"  Total R orbs: {total_r}")
        print(f"  3x3 complete: {'YES' if result['is_complete'] else 'NO'}")
        print()
    
    print(f"Possible cases: {possible_count}/{len(results)}")
    
    if possible_count < len(results):
        print("WARNING: Some cases cannot achieve 3x3 goal!")
    else:
        print("All cases can potentially achieve 3x3 goal with sufficient moves.")

if __name__ == "__main__":
    main()
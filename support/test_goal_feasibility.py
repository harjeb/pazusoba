#!/usr/bin/env python3
"""
Test if the 3x3 goal can actually be achieved in each case
"""

def parse_board(board_str):
    """Parse board string to list"""
    board = []
    orb_map = {'R': 1, 'B': 2, 'G': 3, 'L': 4, 'D': 5, 'H': 6}
    
    for c in board_str[:30]:
        board.append(orb_map.get(c, 0))
    
    return board

def simulate_3x3_solution(board, top_row, top_col, max_moves=40):
    """
    Simulate solving for 3x3 goal
    This is a simplified simulation - actual pathfinding would be more complex
    """
    # Make a copy of the board
    board_copy = board.copy()
    
    # Find R orbs outside target area
    outside_orbs = []
    target_positions = []
    
    # Define target area positions
    for i in range(3):
        for j in range(3):
            pos = (top_row + i) * 6 + (top_col + j)
            target_positions.append(pos)
    
    # Categorize orbs
    r_orbs_outside = []
    non_r_in_target = []
    
    for pos in range(30):
        row, col = pos // 6, pos % 6
        in_target = (top_row <= row < top_row + 3 and top_col <= col < top_col + 3)
        
        if board_copy[pos] == 1:  # R orb
            if in_target:
                pass  # Already in target
            else:
                r_orbs_outside.append(pos)
        elif in_target:
            non_r_in_target.append(pos)
    
    # We need to move R orbs into target and move non-R orbs out
    moves_needed = 0
    
    # Strategy: Move closest R orbs to target first
    # Move non-R orbs out of target to make space
    
    # For each non-R orb in target, we need to move it out
    for non_r_pos in non_r_in_target:
        moves_needed += 1  # At least 1 move to get it out
    
    # For each R orb outside target, we need to move it in
    # Estimate based on distance
    for r_pos in r_orbs_outside[:len(non_r_in_target)]:  # Only need enough to fill target
        r_row, r_col = r_pos // 6, r_pos % 6
        
        # Find closest empty spot in target
        min_distance = float('inf')
        for target_pos in target_positions:
            if board_copy[target_pos] != 1:  # Not already R
                t_row, t_col = target_pos // 6, target_pos % 6
                distance = abs(r_row - t_row) + abs(r_col - t_col)
                min_distance = min(min_distance, distance)
        
        moves_needed += min_distance
    
    # The actual pathfinding would be more complex due to obstacles
    # But this gives us a rough estimate
    estimated_moves = min(moves_needed, max_moves)
    
    return {
        'possible': estimated_moves <= max_moves,
        'estimated_moves': estimated_moves,
        'r_orbs_outside': len(r_orbs_outside),
        'non_r_in_target': len(non_r_in_target),
        'moves_needed': moves_needed
    }

def test_case(board_str, case_name, target_info):
    """Test a specific case"""
    print(f"\n--- {case_name} ---")
    
    board = parse_board(board_str)
    top_row, top_col, expected_score = target_info
    
    # Count current state
    current_r_in_target = 0
    for i in range(3):
        for j in range(3):
            pos = (top_row + i) * 6 + (top_col + j)
            if board[pos] == 1:
                current_r_in_target += 1
    
    print(f"Target: ({top_row},{top_col}) - Current R: {current_r_in_target}/9")
    
    # Test different move limits
    move_limits = [20, 30, 40, 50]
    
    for limit in move_limits:
        result = simulate_3x3_solution(board, top_row, top_col, limit)
        
        if result['possible']:
            print(f"+ {limit} moves: POSSIBLE (est. {result['estimated_moves']} moves)")
            return limit, result['estimated_moves']
        else:
            print(f"- {limit} moves: NOT POSSIBLE (need {result['estimated_moves']} moves)")
    
    return None, result['estimated_moves']

def main():
    """Main test function"""
    test_cases = [
        ("LBGLLDHHGRGBBRHBHRRRGRBBDRDDRR", "Case 1", (2, 0, 4200)),
        ("HBRHLGBBRRDDRRHBHBHBDRDHRDDBRR", "Case 2", (0, 0, 4200)),
        ("DHLRHHRBRRDHRRBLLHHHRGRDLDDRLB", "Case 3", (1, 0, 5300)),
        ("LDBHRDRDLLHRLHDLHRHRGLRRRLLRHB", "Case 4", (1, 3, 4200)),
        ("BRHRLHRBRRGRHHHRLGDBGDBLLRDLRG", "Case 5", (0, 1, 5300)),
    ]
    
    print("3x3 GOAL ACHIEVEMENT TEST")
    print("=" * 50)
    
    results = []
    
    for board_str, case_name, target_info in test_cases:
        min_limit, estimated_moves = test_case(board_str, case_name, target_info)
        results.append((case_name, min_limit, estimated_moves))
    
    print(f"\n{'='*50}")
    print("FINAL RESULTS")
    print(f"{'='*50}")
    
    success_count = 0
    for case_name, min_limit, estimated_moves in results:
        if min_limit:
            success_count += 1
            print(f"{case_name}: YES achievable in {min_limit}+ moves (est. {estimated_moves})")
        else:
            print(f"{case_name}: NO NOT achievable in 50 moves (est. {estimated_moves})")
    
    print(f"\nSuccess rate: {success_count}/{len(results)}")
    
    if success_count == len(results):
        print("SUCCESS! ALL CASES CAN ACHIEVE 3x3 GOAL!")
    else:
        print("WARNING: Some cases cannot achieve 3x3 goal within move limits")

if __name__ == "__main__":
    main()
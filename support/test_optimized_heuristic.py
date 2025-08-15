#!/usr/bin/env python3
"""
Test the optimized 3x3 heuristic algorithm
"""

def calculate_3x3_target_score(board, top_row, top_col, orb_type):
    """Calculate score for a specific 3x3 position"""
    score = 0
    matches = 0
    wrong_orbs = 0
    
    # Count target area status
    for i in range(3):
        for j in range(3):
            pos = (top_row + i) * 6 + (top_col + j)
            if board[pos] == orb_type:
                matches += 1
            elif board[pos] != 0:
                wrong_orbs += 1
    
    # Calculate score
    score += matches * 1000  # Matched orbs
    score -= wrong_orbs * 100  # Penalty for wrong orbs
    
    # Cluster bonus
    if matches >= 6:
        score += 2000
    elif matches >= 4:
        score += 500
    
    # Boundary bonus
    if top_row == 0 or top_col == 0 or top_row == 2 or top_col == 3:
        score += 200
    
    return score, matches

def calculate_distance_bonus(board, top_row, top_col, orb_type):
    """Calculate distance bonus for orbs outside target area"""
    total_distance = 0
    target_orb_count = 0
    
    for i in range(30):
        if board[i] == orb_type:
            target_orb_count += 1
            row = i // 6
            col = i % 6
            
            # Check if in target area
            in_target = (row >= top_row and row < top_row + 3 and
                        col >= top_col and col < top_col + 3)
            
            if not in_target:
                # Calculate distance to target center
                center_row = top_row + 1
                center_col = top_col + 1
                distance = abs(row - center_row) + abs(col - center_col)
                total_distance += distance
    
    distance_bonus = max(0, 1000 - total_distance * 10)
    density_bonus = target_orb_count * 500
    
    return distance_bonus, density_bonus, target_orb_count

def optimized_heuristic_evaluate(board, top_row, top_col, orb_type):
    """Optimized heuristic evaluation"""
    # Base score from target
    base_score, matches = calculate_3x3_target_score(board, top_row, top_col, orb_type)
    
    # Distance and density bonuses
    distance_bonus, density_bonus, target_orb_count = calculate_distance_bonus(
        board, top_row, top_col, orb_type)
    
    # Progressive rewards based on current matches
    progressive_bonus = 0
    if matches >= 8:
        progressive_bonus = 50000
    elif matches >= 7:
        progressive_bonus = 30000
    elif matches >= 6:
        progressive_bonus = 15000
    elif matches >= 5:
        progressive_bonus = 8000
    elif matches >= 4:
        progressive_bonus = 4000
    
    # Total score
    total_score = base_score + distance_bonus + density_bonus + progressive_bonus
    
    # Check for perfect 3x3
    perfect_3x3 = (matches == 9)
    if perfect_3x3:
        total_score += 200000
    
    return total_score, matches, perfect_3x3, base_score

def parse_board(board_str):
    """Parse board string to list"""
    board = []
    orb_map = {'R': 1, 'B': 2, 'G': 3, 'L': 4, 'D': 5, 'H': 6}
    
    for c in board_str[:30]:
        board.append(orb_map.get(c, 0))
    
    return board

def print_board_area(board, top_row, top_col, orb_type):
    """Print the target area"""
    print("Target area layout:")
    orb_symbols = ['.', 'R', 'B', 'G', 'L', 'D', 'H']
    
    for i in range(3):
        print("  ", end="")
        for j in range(3):
            pos = (top_row + i) * 6 + (top_col + j)
            orb_char = orb_symbols[board[pos]]
            if board[pos] == orb_type:
                print(f"[{orb_char}]", end="")
            else:
                print(f" {orb_char} ", end="")
        print()

def test_case(board_str, case_name):
    """Test a specific case"""
    print(f"\n=== {case_name} ===")
    print(f"Board: {board_str}")
    
    board = parse_board(board_str)
    
    # Find best 3x3 target (simplified - just check R orbs)
    best_score = -1
    best_target = None
    
    for top_row in range(3):
        for top_col in range(4):
            score, matches = calculate_3x3_target_score(board, top_row, top_col, 1)
            if score > best_score:
                best_score = score
                best_target = (top_row, top_col, matches)
    
    if best_target:
        top_row, top_col, initial_matches = best_target
        
        print(f"Best 3x3 target: Orb 1 at ({top_row},{top_col}), Base Score: {best_score}")
        print_board_area(board, top_row, top_col, 1)
        
        # Test optimized heuristic
        total_score, matches, perfect_3x3, base_score = optimized_heuristic_evaluate(
            board, top_row, top_col, 1)
        
        print(f"Initial matches in target area: {initial_matches}")
        print(f"Optimized evaluation:")
        print(f"  Total Score: {total_score}")
        print(f"  Base Score: {base_score}")
        print(f"  Perfect 3x3: {'YES' if perfect_3x3 else 'NO'}")
        print(f"  Current matches: {matches}")
        
        # Calculate target score threshold (original was 4200)
        target_threshold = 4200
        goal_achieved = total_score >= target_threshold
        
        print(f"Target threshold: {target_threshold}")
        print(f"Goal achieved: {'YES' if goal_achieved else 'NO'}")
        
        return goal_achieved, total_score
    
    return False, 0

def main():
    """Test all cases"""
    test_cases = [
        ("LBGLLDHHGRGBBRHBHRRRGRBBDRDDRR", "Case 1"),
        ("HBRHLGBBRRDDRRHBHBHBDRDHRDDBRR", "Case 2"),
        ("DHLRHHRBRRDHRRBLLHHHRGRDLDDRLB", "Case 3"),
        ("LDBHRDRDLLHRLHDLHRHRGLRRRLLRHB", "Case 4"),
        ("BRHRLHRBRRGRHHHRLGDBGDBLLRDLRG", "Case 5"),
    ]
    
    results = []
    
    for board_str, case_name in test_cases:
        goal_achieved, score = test_case(board_str, case_name)
        results.append((case_name, goal_achieved, score))
    
    print("\n" + "="*50)
    print("SUMMARY")
    print("="*50)
    
    success_count = 0
    for case_name, goal_achieved, score in results:
        status = "PASS" if goal_achieved else "FAIL"
        print(f"{case_name}: {status} (Score: {score})")
        if goal_achieved:
            success_count += 1
    
    print(f"\nSuccess rate: {success_count}/{len(results)} ({success_count/len(results)*100:.1f}%)")

if __name__ == "__main__":
    main()
#!/usr/bin/env python3
"""
Test the 10 scattered cases for 3x3 goal achievement
"""

def parse_board(board_str):
    """Parse board string to list"""
    board = []
    orb_map = {'R': 1, 'B': 2, 'G': 3, 'L': 4, 'D': 5, 'H': 6}
    
    for c in board_str[:30]:
        board.append(orb_map.get(c, 0))
    
    return board

def simulate_3x3_solution(board, top_row, top_col, target_orb_type, max_moves=40):
    """Simulate solving for 3x3 goal"""
    # Find target orbs outside target area
    target_positions = []
    
    # Define target area positions
    for i in range(3):
        for j in range(3):
            pos = (top_row + i) * 6 + (top_col + j)
            target_positions.append(pos)
    
    # Categorize orbs
    target_orbs_outside = []
    non_target_in_target = []
    
    for pos in range(30):
        row, col = pos // 6, pos % 6
        in_target = (top_row <= row < top_row + 3 and top_col <= col < top_col + 3)
        
        if board[pos] == target_orb_type:  # Target orb
            if in_target:
                pass  # Already in target
            else:
                target_orbs_outside.append(pos)
        elif in_target:
            non_target_in_target.append(pos)
    
    # Calculate moves needed
    moves_needed = 0
    
    # Move non-target orbs out of target
    for non_target_pos in non_target_in_target:
        moves_needed += 1
    
    # Move target orbs into target
    for target_pos in target_orbs_outside[:len(non_target_in_target)]:
        target_row, target_col = target_pos // 6, target_pos % 6
        
        # Find closest empty spot in target
        min_distance = float('inf')
        for target_area_pos in target_positions:
            if board[target_area_pos] != target_orb_type:  # Not already target orb
                t_row, t_col = target_area_pos // 6, target_area_pos % 6
                distance = abs(target_row - t_row) + abs(target_col - t_col)
                min_distance = min(min_distance, distance)
        
        moves_needed += min_distance
    
    return {
        'possible': moves_needed <= max_moves,
        'estimated_moves': moves_needed,
        'target_orbs_outside': len(target_orbs_outside),
        'non_target_in_target': len(non_target_in_target),
        'current_in_target': 9 - len(target_orbs_outside)
    }

def find_best_3x3_target(board, orb_type):
    """Find the best 3x3 target area for given orb type"""
    best_target = None
    best_count = 0
    
    for top_row in range(3):
        for top_col in range(4):
            count = 0
            for i in range(3):
                for j in range(3):
                    pos = (top_row + i) * 6 + (top_col + j)
                    if board[pos] == orb_type:
                        count += 1
            
            if count > best_count:
                best_count = count
                best_target = (top_row, top_col)
    
    return best_target, best_count

def test_scattered_case(board_str, case_num, target_orb_type):
    """Test a scattered case"""
    print(f"\n--- Scattered Case {case_num} ---")
    
    board = parse_board(board_str)
    
    orb_names = {1: 'R', 2: 'B', 3: 'G', 4: 'L', 5: 'D', 6: 'H'}
    orb_name = orb_names[target_orb_type]
    
    print(f"Target orb: {orb_name}")
    
    # Find best target area
    best_target, best_count = find_best_3x3_target(board, target_orb_type)
    
    if not best_target:
        print("ERROR: No valid target area found")
        return None
    
    top_row, top_col = best_target
    print(f"Best target: ({top_row},{top_col}) with {best_count}/9 {orb_name} orbs")
    
    # Test with different move limits
    move_limits = [15, 20, 25, 30, 35, 40]
    
    for limit in move_limits:
        result = simulate_3x3_solution(board, top_row, top_col, target_orb_type, limit)
        
        if result['possible']:
            print(f"+ {limit} moves: POSSIBLE (est. {result['estimated_moves']} moves)")
            return limit, result['estimated_moves'], result['current_in_target']
        else:
            print(f"- {limit} moves: NOT POSSIBLE (need {result['estimated_moves']} moves)")
    
    return None, result['estimated_moves'], result['current_in_target']

def main():
    """Test all 10 scattered cases"""
    # The 10 generated cases (from saved file)
    test_cases = [
        ("BDGRHLHRDLRLDHLLDHDBHHHLBGHRHD", 6),  # Case 1: H orbs
        ("DDBBBRRHLHRRLGHBGRDLLHRHDRBRRG", 1),  # Case 2: R orbs
        ("DHBHBGBHLBDHLLDBGGRHLLBBLBRRHB", 2),  # Case 3: B orbs
        ("HDHHRDHGHDRDBBDGHHDHGDLDBHRDHL", 5),  # Case 4: D orbs
        ("BBDDGDRDDGHRBHRRBDBBGRGRBBDHBR", 2),  # Case 5: B orbs
        ("GHRBGDBHGBDGRLGHGRBHRLBGGDHLGB", 3),  # Case 6: G orbs
        ("GLDLGRGLRDDGBGGBDDHLHLGLDGGDHB", 3),  # Case 7: G orbs
        ("RBRRRGGGGHLGHRHGHHBHRGHHRGHGLD", 3),  # Case 8: G orbs
        ("DLRLHRGRBHDRBHLRRBRDRDHGLHLRHH", 1),  # Case 9: R orbs
        ("BBDGGGHHBRRBHHGRGRLHHBHGLHHBBD", 6),  # Case 10: H orbs
    ]
    
    print("TESTING 10 SCATTERED CASES FOR 3x3 GOAL")
    print("=" * 60)
    
    results = []
    
    for i, (board_str, target_orb_type) in enumerate(test_cases):
        result = test_scattered_case(board_str, i + 1, target_orb_type)
        if result:
            min_moves, estimated_moves, current_in_target = result
            results.append({
                'case': i + 1,
                'min_moves': min_moves,
                'estimated_moves': estimated_moves,
                'current_in_target': current_in_target
            })
        else:
            results.append({
                'case': i + 1,
                'min_moves': None,
                'estimated_moves': 0,  # Default value
                'current_in_target': 0
            })
    
    # Summary
    print(f"\n{'='*60}")
    print("SCATTERED CASES RESULTS SUMMARY")
    print(f"{'='*60}")
    
    success_count = 0
    total_estimated_moves = 0
    
    for result in results:
        case_num = result['case']
        min_moves = result['min_moves']
        estimated_moves = result['estimated_moves']
        current_in_target = result['current_in_target']
        
        if min_moves:
            success_count += 1
            total_estimated_moves += estimated_moves
            print(f"Case {case_num}: YES in {min_moves}+ moves (est. {estimated_moves}, current: {current_in_target}/9)")
        else:
            print(f"Case {case_num}: NO in 40 moves (est. {estimated_moves}, current: {current_in_target}/9)")
    
    print(f"\nSuccess rate: {success_count}/10 ({success_count/10*100:.1f}%)")
    
    if success_count > 0:
        avg_moves = total_estimated_moves / success_count
        print(f"Average estimated moves for successful cases: {avg_moves:.1f}")
    
    # Difficulty classification
    print(f"\nDifficulty breakdown:")
    easy_count = sum(1 for r in results if r['min_moves'] and r['estimated_moves'] <= 20)
    medium_count = sum(1 for r in results if r['min_moves'] and 20 < r['estimated_moves'] <= 30)
    hard_count = sum(1 for r in results if r['min_moves'] and r['estimated_moves'] > 30)
    
    print(f"Easy (≤20 moves): {easy_count} cases")
    print(f"Medium (21-30 moves): {medium_count} cases")
    print(f"Hard (>30 moves): {hard_count} cases")
    print(f"Impossible: {10 - success_count} cases")

if __name__ == "__main__":
    main()
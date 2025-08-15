#!/usr/bin/env python3
"""
Generate 10 new scattered test cases with 9 identical orbs each
"""

import random

def generate_scattered_board():
    """Generate a board with 9 identical orbs scattered around"""
    # Choose orb type (1-6: R,B,G,L,D,H)
    orb_type = random.randint(1, 6)
    
    # Create empty board
    board = [0] * 30
    
    # Generate 9 random positions for the target orb
    positions = random.sample(range(30), 9)
    
    # Place the target orbs
    for pos in positions:
        board[pos] = orb_type
    
    # Fill remaining positions with other random orbs
    other_orbs = [i for i in range(1, 7) if i != orb_type]
    for i in range(30):
        if board[i] == 0:
            board[i] = random.choice(other_orbs)
    
    # Convert to string representation
    orb_map = {1: 'R', 2: 'B', 3: 'G', 4: 'L', 5: 'D', 6: 'H'}
    board_str = ''.join([orb_map[orb] for orb in board])
    
    return board_str, orb_type, positions

def print_board(board_str, title=""):
    """Print the board"""
    if title:
        print(f"\n{title}")
    
    for i in range(5):
        row = ""
        for j in range(6):
            pos = i * 6 + j
            row += board_str[pos] + " "
        print(row)

def analyze_scattering(board_str, orb_type, positions):
    """Analyze how scattered the orbs are"""
    # Calculate average distance from center
    center_row, center_col = 2, 2.5  # Board center
    total_distance = 0
    
    for pos in positions:
        row, col = pos // 6, pos % 6
        distance = ((row - center_row) ** 2 + (col - center_col) ** 2) ** 0.5
        total_distance += distance
    
    avg_distance = total_distance / 9
    
    # Find best 3x3 target area
    best_target = None
    best_count = 0
    
    for top_row in range(3):
        for top_col in range(4):
            count = 0
            for i in range(3):
                for j in range(3):
                    target_pos = (top_row + i) * 6 + (top_col + j)
                    if target_pos in positions:
                        count += 1
            
            if count > best_count:
                best_count = count
                best_target = (top_row, top_col)
    
    return avg_distance, best_target, best_count

def main():
    """Generate 10 scattered test cases"""
    print("GENERATING 10 SCATTERED TEST CASES")
    print("=" * 60)
    
    test_cases = []
    
    for i in range(10):
        board_str, orb_type, positions = generate_scattered_board()
        avg_distance, best_target, best_count = analyze_scattering(board_str, orb_type, positions)
        
        test_cases.append({
            'board_str': board_str,
            'orb_type': orb_type,
            'positions': positions,
            'avg_distance': avg_distance,
            'best_target': best_target,
            'best_count': best_count
        })
        
        print(f"\n--- Case {i+1} ---")
        print(f"Orb type: {['R','B','G','L','D','H'][orb_type-1]}")
        print(f"Scattering: avg distance = {avg_distance:.2f}")
        print(f"Best 3x3 target: {best_target} with {best_count}/9 orbs")
        print_board(board_str, f"Board {i+1}:")
    
    # Save test cases to file for later testing
    with open('D:/pycode/core/support/scattered_test_cases.txt', 'w') as f:
        f.write("10 SCATTERED TEST CASES\n")
        f.write("=" * 60 + "\n\n")
        
        for i, case in enumerate(test_cases):
            f.write(f"Case {i+1}:\n")
            f.write(f"Board: {case['board_str']}\n")
            f.write(f"Orb type: {case['orb_type']}\n")
            f.write(f"Best target: {case['best_target']}\n")
            f.write(f"Orbs in target: {case['best_count']}/9\n")
            f.write(f"Avg scattering distance: {case['avg_distance']:.2f}\n")
            f.write("\n")
    
    print(f"\n{'='*60}")
    print("TEST CASES GENERATED AND SAVED")
    print(f"File: D:/pycode/core/support/scattered_test_cases.txt")
    print(f"{'='*60}")
    
    return test_cases

if __name__ == "__main__":
    test_cases = main()
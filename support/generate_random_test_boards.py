#!/usr/bin/env python3
"""
Generate 5 random test boards with exactly 9 orbs of one color, distributed randomly
"""

import random

def generate_random_scattered_board():
    """Generate a random board with exactly 9 R's scattered randomly"""
    
    # Create base board with other colors
    other_colors = ['B', 'G', 'L', 'D', 'H']
    board = []
    
    # Fill all 30 positions with random other colors first
    for i in range(30):
        board.append(random.choice(other_colors))
    
    # Now randomly replace 9 positions with R
    r_positions = random.sample(range(30), 9)  # Select 9 random positions
    
    for pos in r_positions:
        board[pos] = 'R'
    
    return ''.join(board), r_positions

def analyze_board_3x3_potential(board_string, target_orb='R'):
    """Analyze the 3x3 potential of a board"""
    print(f"\nBoard: {board_string}")
    
    # Show board layout
    print("Board layout:")
    for row in range(5):
        line = "  "
        for col in range(6):
            pos = row * 6 + col
            orb = board_string[pos]
            if orb == target_orb:
                line += f"[{orb}]"
            else:
                line += f" {orb} "
        print(line)
    
    # Count target orbs
    target_count = board_string.count(target_orb)
    print(f"\nTarget orb '{target_orb}' count: {target_count}")
    
    # Find positions of target orb
    target_positions = [i for i, orb in enumerate(board_string) if orb == target_orb]
    print(f"Positions: {target_positions}")
    
    # Analyze all possible 3x3 areas
    print("\n3x3 Area Analysis:")
    best_score = 0
    best_area = None
    
    for top_row in range(3):  # 0, 1, 2
        for top_col in range(4):  # 0, 1, 2, 3
            target_count_in_area = 0
            area_positions = []
            
            # Check this 3x3 area
            for i in range(3):
                for j in range(3):
                    pos = (top_row + i) * 6 + (top_col + j)
                    area_positions.append(pos)
                    if board_string[pos] == target_orb:
                        target_count_in_area += 1
            
            if target_count_in_area >= 1:  # Only show areas with at least 1 target orb
                print(f"  Area ({top_row},{top_col}): {target_count_in_area}/9 {target_orb}'s")
                print(f"    Positions: {area_positions}")
                
                if target_count_in_area > best_score:
                    best_score = target_count_in_area
                    best_area = (top_row, top_col)
    
    if best_area:
        print(f"\nBest 3x3 area: ({best_area[0]},{best_area[1]}) with {best_score}/9 {target_orb}'s")
    
    return best_score, best_area

def main():
    print("="*70)
    print("GENERATING 5 RANDOM TEST BOARDS WITH 9 R's EACH")
    print("="*70)
    
    test_boards = []
    
    for i in range(5):
        print(f"\n{'='*20} TEST CASE {i+1} {'='*20}")
        
        # Generate random board
        board, r_positions = generate_random_scattered_board()
        test_boards.append(board)
        
        # Analyze potential
        best_score, best_area = analyze_board_3x3_potential(board)
        
        # Calculate difficulty rating
        if best_score >= 6:
            difficulty = "EASY"
        elif best_score >= 4:
            difficulty = "MEDIUM"
        elif best_score >= 2:
            difficulty = "HARD"
        else:
            difficulty = "VERY HARD"
        
        print(f"\nDifficulty Rating: {difficulty}")
        
        # Show distance analysis (how spread out the R's are)
        min_pos = min(r_positions)
        max_pos = max(r_positions)
        spread = max_pos - min_pos
        print(f"Spread: positions {min_pos}-{max_pos} (range: {spread})")
    
    print("\n" + "="*70)
    print("SUMMARY - BOARDS FOR TESTING")
    print("="*70)
    
    for i, board in enumerate(test_boards, 1):
        print(f"{i}. {board}")
    
    print(f"\nAll boards ready for heuristic algorithm testing!")
    
    return test_boards

if __name__ == "__main__":
    boards = main()
#!/usr/bin/env python3
"""
Create test boards that can form 3x3 squares
"""

import random

def create_3x3_test_board():
    """Create a board where one color can potentially form a 3x3 square"""
    
    # Strategy: Place 9 R's strategically, then fill with other colors
    board = [' '] * 30
    
    # Test case 1: 9 R's scattered but can potentially be gathered into 3x3
    # Place them in positions that could form a square with some moves
    r_positions = [0, 1, 2, 6, 7, 8, 12, 13, 14]  # Perfect 3x3 in top-left
    
    # But let's make it more challenging - scatter some R's
    r_positions = [0, 2, 4, 7, 9, 12, 15, 18, 20, 25]  # 10 R's scattered
    r_positions = r_positions[:9]  # Take only 9
    
    for pos in r_positions:
        board[pos] = 'R'
    
    # Fill remaining positions with other colors
    other_colors = ['B', 'G', 'L', 'D', 'H']
    remaining_positions = [i for i in range(30) if board[i] == ' ']
    
    for pos in remaining_positions:
        board[pos] = random.choice(other_colors)
    
    return ''.join(board)

def create_challenging_3x3_board():
    """Create a more challenging board for 3x3 formation"""
    
    # This board will have exactly 9 D's that need to be moved to form a 3x3
    board = list("BGLDHBRGLGRHBRGLBHBRGLDHBRGLHR")  # Base random board
    
    # Replace some positions with D to get exactly 9 D's
    d_positions = [1, 3, 5, 8, 11, 14, 17, 20, 23]  # 9 D's in scattered positions
    
    # Reset board first
    board = ['B'] * 30
    colors = ['B', 'G', 'L', 'H', 'R']
    
    # Fill with random colors first
    for i in range(30):
        board[i] = random.choice(colors)
    
    # Now place exactly 9 D's
    for pos in d_positions:
        board[pos] = 'D'
    
    return ''.join(board)

def create_near_3x3_board():
    """Create a board where 3x3 is almost formed"""
    board = [' '] * 30
    
    # Place 8 R's in almost-3x3 formation (missing one corner)
    almost_3x3 = [0, 1, 2, 6, 7, 8, 12, 13]  # Missing position 14
    
    # Add one more R elsewhere
    almost_3x3.append(20)
    
    for pos in almost_3x3:
        board[pos] = 'R'
    
    # Fill remaining with other colors
    other_colors = ['B', 'G', 'L', 'D', 'H']
    for i in range(30):
        if board[i] == ' ':
            board[i] = random.choice(other_colors)
    
    return ''.join(board)

def print_board_analysis(board_string, name):
    """Print board analysis"""
    print(f"\n=== {name} ===")
    print(f"Board: {board_string}")
    
    # Count colors
    color_count = {}
    for orb in board_string:
        color_count[orb] = color_count.get(orb, 0) + 1
    
    print("Color count:")
    for color, count in sorted(color_count.items()):
        marker = " ★" if count >= 9 else ""
        print(f"  {color}: {count}{marker}")
    
    # Show board layout
    print("\nBoard layout:")
    for i in range(5):
        row = ""
        for j in range(6):
            idx = i * 6 + j
            row += f" {board_string[idx]}"
        print(f"  {row}")

def main():
    # Create different test boards
    board1 = create_near_3x3_board()
    board2 = create_challenging_3x3_board() 
    
    # Create a more targeted board
    board3 = "RRRBBBGGGRRRBBBGGGRRRBBBGGGBGH"  # 9 R's, 9 B's, 9 G's
    
    # Create scattered 9-orb board
    board4 = "RBRGRBRGRBRGRBRGRBRGRBRGRBRGRB"  # 13 R's, need to select 9
    board4 = "RBRGRBRGRBRGRBRGRBRGRBRGRBRGRL"  # 12 R's
    board4 = "RBGRBGRBGRBGRBGRBGRBGRBGRBGLHD"  # 9 R's scattered
    
    print_board_analysis(board1, "NEAR 3x3 BOARD (8+1 R)")
    print_board_analysis(board2, "CHALLENGING BOARD (9 D)")
    print_board_analysis(board3, "ORGANIZED BOARD (9 R, 9 B, 9 G)")
    print_board_analysis(board4, "SCATTERED BOARD (9 R)")
    
    print("\n" + "="*50)
    print("RECOMMENDED TEST BOARDS FOR 3x3:")
    print("="*50)
    print(f"1. Near-perfect: {board1}")
    print(f"2. Challenging:  {board2}")
    print(f"3. Organized:    {board3}")
    print(f"4. Scattered:    {board4}")

if __name__ == "__main__":
    main()
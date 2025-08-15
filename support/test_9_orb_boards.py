#!/usr/bin/env python3
"""
Create test boards with exactly 9 orbs of one color for 3x3 testing
"""

def create_test_boards():
    """Create various test boards with 9 orbs of one color"""
    
    # Test 1: 9 R's almost in 3x3 formation (missing just one position)
    board1 = list("BGLDHBGLDHBGLDHBGLDHBGLDHBGLDH")
    # Place 8 R's in top-left 3x3 area, missing position 14 (bottom-right of 3x3)
    r_positions = [0, 1, 2, 6, 7, 8, 12, 13, 20]  # 9 R's, 8 in almost-3x3
    for pos in r_positions:
        board1[pos] = 'R'
    board1_str = ''.join(board1)
    
    # Test 2: 9 R's scattered evenly across the board
    board2 = list("BGLDHBGLDHBGLDHBGLDHBGLDHBGLDH")
    r_positions_scattered = [0, 3, 6, 9, 12, 15, 18, 21, 24]  # Every 3rd position
    for pos in r_positions_scattered:
        board2[pos] = 'R'
    board2_str = ''.join(board2)
    
    # Test 3: 9 R's in corners and edges (hardest to form 3x3)
    board3 = list("BGLDHBGLDHBGLDHBGLDHBGLDHBGLDH")
    r_positions_corners = [0, 2, 4, 6, 11, 18, 23, 24, 29]  # Corners and edges
    for pos in r_positions_corners:
        board3[pos] = 'R'
    board3_str = ''.join(board3)
    
    # Test 4: 9 R's with 6 already in a 2x3 block (easier to extend)
    board4 = list("BGLDHBGLDHBGLDHBGLDHBGLDHBGLDH")
    r_positions_block = [0, 1, 6, 7, 12, 13, 18, 25, 28]  # 6 in 2x3, 3 scattered
    for pos in r_positions_block:
        board4[pos] = 'R'
    board4_str = ''.join(board4)
    
    # Test 5: Perfect 3x3 with one wrong orb in the middle
    board5 = list("BGLDHBGLDHBGLDHBGLDHBGLDHBGLDH")
    perfect_3x3 = [0, 1, 2, 6, 7, 8, 12, 13, 14]
    board5[7] = 'G'  # Make position 7 wrong
    r_positions_perfect = [0, 1, 2, 6, 8, 12, 13, 14, 20]  # 8 in perfect 3x3, 1 elsewhere
    for pos in r_positions_perfect:
        board5[pos] = 'R'
    board5_str = ''.join(board5)
    
    return [
        ("Almost 3x3 (8+1)", board1_str),
        ("Scattered evenly", board2_str), 
        ("Corners & edges", board3_str),
        ("2x3 block + 3", board4_str),
        ("Perfect-1 + 1", board5_str)
    ]

def print_board_analysis(name, board_str):
    """Print board analysis"""
    print(f"\n=== {name} ===")
    print(f"Board: {board_str}")
    
    # Count colors
    color_count = {}
    for orb in board_str:
        color_count[orb] = color_count.get(orb, 0) + 1
    
    print("Color count:")
    for color, count in sorted(color_count.items()):
        marker = " ★" if count >= 9 else ""
        print(f"  {color}: {count}{marker}")
    
    # Show R positions
    r_positions = [i for i, orb in enumerate(board_str) if orb == 'R']
    print(f"R positions: {r_positions}")
    
    # Show board layout
    print("Board layout:")
    for i in range(5):
        row = ""
        for j in range(6):
            idx = i * 6 + j
            orb = board_str[idx]
            if orb == 'R':
                row += f" [R]"  # Highlight R's
            else:
                row += f"  {orb} "
        print(f"  {row}")
        
    # Check how close to 3x3
    print("\n3x3 potential analysis:")
    for top_row in range(3):  # 0,1,2 (can fit 3x3)
        for top_col in range(4):  # 0,1,2,3 (can fit 3x3)
            r_count = 0
            positions = []
            for i in range(3):
                for j in range(3):
                    pos = (top_row + i) * 6 + (top_col + j)
                    positions.append(pos)
                    if board_str[pos] == 'R':
                        r_count += 1
            if r_count >= 2:  # Only show promising areas
                print(f"  3x3 at ({top_row},{top_col}): {r_count}/9 R's, positions {positions}")

def main():
    boards = create_test_boards()
    
    print("="*60)
    print("TEST BOARDS WITH EXACTLY 9 R ORBS")
    print("="*60)
    
    for name, board_str in boards:
        print_board_analysis(name, board_str)
    
    print("\n" + "="*60)
    print("READY FOR TESTING WITH FORCE 3X3 SOLVER")
    print("="*60)
    
    for i, (name, board_str) in enumerate(boards, 1):
        print(f"{i}. {name}: {board_str}")

if __name__ == "__main__":
    main()
#!/usr/bin/env python3
"""
Analyze board for 3x3 square formation strategy
"""

def analyze_board_colors(board_string):
    """Analyze color distribution in the board"""
    print(f"Board: {board_string}")
    print("\n=== COLOR ANALYSIS ===")
    
    # Count each orb type
    color_count = {}
    for orb in board_string:
        color_count[orb] = color_count.get(orb, 0) + 1
    
    print("Color distribution:")
    for color, count in sorted(color_count.items()):
        print(f"  {color}: {count} orbs")
        if count >= 9:
            print(f"    *** {color} has {count} orbs - can form 3x3! ***")
    
    return color_count

def show_board_positions(board_string, rows=5, cols=6):
    """Display board with position indices"""
    print("\n=== BOARD LAYOUT ===")
    print("Position indices:")
    for i in range(rows):
        row_positions = []
        row_orbs = []
        for j in range(cols):
            idx = i * cols + j
            row_positions.append(f"{idx:2d}")
            row_orbs.append(f" {board_string[idx]}")
        print(" ".join(row_positions))
        print(" ".join(row_orbs))
        print()

def find_orb_positions(board_string, target_orb, rows=5, cols=6):
    """Find all positions of a specific orb"""
    positions = []
    for i, orb in enumerate(board_string):
        if orb == target_orb:
            row, col = divmod(i, cols)
            positions.append((i, row, col))
    return positions

def analyze_3x3_potential(board_string, target_orb, rows=5, cols=6):
    """Analyze potential for forming 3x3 squares"""
    positions = find_orb_positions(board_string, target_orb, rows, cols)
    print(f"\n=== 3x3 POTENTIAL ANALYSIS FOR {target_orb} ===")
    print(f"Current {target_orb} positions:")
    for idx, row, col in positions:
        print(f"  Index {idx}: ({row},{col})")
    
    # Check all possible 3x3 top-left corners
    possible_squares = []
    for top_row in range(rows - 2):
        for top_col in range(cols - 2):
            square_positions = []
            for i in range(3):
                for j in range(3):
                    pos_idx = (top_row + i) * cols + (top_col + j)
                    square_positions.append(pos_idx)
            
            # Count how many positions already have the target orb
            current_count = sum(1 for pos in square_positions if board_string[pos] == target_orb)
            needed_moves = 9 - current_count
            
            possible_squares.append({
                'top_left': (top_row, top_col),
                'positions': square_positions,
                'current_count': current_count,
                'needed_moves': needed_moves
            })
    
    # Sort by fewest moves needed
    possible_squares.sort(key=lambda x: x['needed_moves'])
    
    print(f"\nPossible 3x3 squares (sorted by ease):")
    for i, square in enumerate(possible_squares[:3]):  # Show top 3
        top_row, top_col = square['top_left']
        print(f"\n  Option {i+1}: Top-left at ({top_row},{top_col})")
        print(f"    Positions: {square['positions']}")
        print(f"    Current {target_orb} count: {square['current_count']}/9")
        print(f"    Moves needed: {square['needed_moves']}")
        
        # Show current state of this 3x3 area
        print(f"    Current 3x3 area:")
        for row_offset in range(3):
            row_str = ""
            for col_offset in range(3):
                pos = (top_row + row_offset) * cols + (top_col + col_offset)
                orb = board_string[pos]
                row_str += f" {orb}"
            print(f"      {row_str}")
    
    return possible_squares

def suggest_3x3_strategy(board_string):
    """Suggest the best strategy for creating a 3x3 square"""
    print("\n" + "="*50)
    print("3X3 SQUARE FORMATION STRATEGY")
    print("="*50)
    
    color_count = analyze_board_colors(board_string)
    show_board_positions(board_string)
    
    # Find colors with 9+ orbs
    candidates = [color for color, count in color_count.items() if count >= 9]
    
    if not candidates:
        print("\nNo color has 9+ orbs. Cannot form 3x3 square!")
        return None
    
    print(f"\nColors that can form 3x3: {candidates}")
    
    best_strategies = []
    for color in candidates:
        squares = analyze_3x3_potential(board_string, color)
        if squares:
            best_strategies.append((color, squares[0]))  # Best option for this color
    
    # Find the overall best strategy
    if best_strategies:
        best_strategies.sort(key=lambda x: x[1]['needed_moves'])
        best_color, best_square = best_strategies[0]
        
        print(f"\n" + "="*30)
        print(f"RECOMMENDED STRATEGY")
        print(f"="*30)
        print(f"Target color: {best_color}")
        print(f"Target 3x3 area: Top-left at {best_square['top_left']}")
        print(f"Positions needed: {best_square['positions']}")
        print(f"Moves required: {best_square['needed_moves']}")
        
        return best_color, best_square
    
    return None

def main():
    # Test with the first example board
    board1 = "DGRRBLHGBBGGRDDDDLBGHDBLLHDBLD"
    suggest_3x3_strategy(board1)
    
    print("\n" + "="*60)
    print("\nTesting second board too:")
    board2 = "BGLDHBRGLGRHBRGLBHBRGLDHBRGLHR"
    suggest_3x3_strategy(board2)

if __name__ == "__main__":
    main()
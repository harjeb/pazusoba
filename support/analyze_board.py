#!/usr/bin/env python3
"""
Analyze the board and find path to form 3x3 square with B orbs
"""

def print_board(board, row, column):
    """Print the board in a readable format"""
    print("Current Board:")
    for i in range(row):
        for j in range(column):
            index = i * column + j
            print(f"{board[index]} ", end=" ")
        print()
    print()

def find_b_positions(board):
    """Find all positions of B orbs"""
    b_positions = []
    for i, orb in enumerate(board):
        if orb == 'B':
            b_positions.append(i)
    return b_positions

def find_potential_3x3_squares(board, row, column):
    """Find all potential 3x3 squares that could be formed with B orbs"""
    potential_squares = []
    
    for r in range(row - 2):
        for c in range(column - 2):
            square_positions = []
            b_count = 0
            
            for i in range(3):
                for j in range(3):
                    index = (r + i) * column + (c + j)
                    square_positions.append(index)
                    if board[index] == 'B':
                        b_count += 1
            
            # If this area has at least 6 B orbs, it's a potential target
            if b_count >= 6:
                potential_squares.append({
                    'top_left': (r, c),
                    'positions': square_positions,
                    'b_count': b_count,
                    'non_b_positions': [pos for pos in square_positions if board[pos] != 'B']
                })
    
    return potential_squares

def analyze_board(board_string):
    """Analyze the given board and find 3x3 square solution"""
    # Parse board
    board = list(board_string)
    
    # Determine board size
    board_size = len(board)
    if board_size == 20:
        row, column = 4, 5
    elif board_size == 28:
        row, column = 4, 7
    elif board_size == 30:
        row, column = 5, 6
    elif board_size == 42:
        row, column = 6, 7
    else:
        print(f"Unsupported board size: {board_size}")
        return
    
    print(f"Board size: {row}x{column}")
    print_board(board, row, column)
    
    # Find all B positions
    b_positions = find_b_positions(board)
    print(f"B orb positions: {b_positions}")
    print(f"Total B orbs: {len(b_positions)}")
    
    # Find potential 3x3 squares
    potential_squares = find_potential_3x3_squares(board, row, column)
    
    if not potential_squares:
        print("No potential 3x3 squares found with 6+ B orbs")
        return
    
    print(f"\nFound {len(potential_squares)} potential 3x3 squares:")
    
    for i, square in enumerate(potential_squares):
        print(f"\nSquare {i+1}: Top-left {square['top_left']}")
        print(f"B count: {square['b_count']}/9")
        print(f"Non-B positions: {square['non_b_positions']}")
        
        # Show what needs to be changed
        print("Required changes:")
        for pos in square['non_b_positions']:
            r, c = pos // column, pos % column
            print(f"  Position ({r}, {c}): {board[pos]} -> B")
    
    # Find the best candidate (most B orbs already)
    best_square = max(potential_squares, key=lambda x: x['b_count'])
    print(f"\n=== BEST CANDIDATE ===")
    print(f"Top-left: {best_square['top_left']}")
    print(f"B orbs already in place: {best_square['b_count']}/9")
    print(f"Need to change {len(best_square['non_b_positions'])} positions to B")
    
    return best_square

def suggest_move_sequence(board_string, target_square):
    """Suggest a sequence of moves to form the 3x3 square"""
    print(f"\n=== SUGGESTED MOVE STRATEGY ===")
    
    # For now, just identify which orbs need to be moved
    non_b_positions = target_square['non_b_positions']
    
    print("Orbs that need to be moved out of the 3x3 area:")
    for pos in non_b_positions:
        r, c = pos // 6, pos % 6  # Assuming 5x6 board
        print(f"  Position ({r}, {c}): {board_string[pos]}")
    
    print("\nStrategy:")
    print("1. Move non-B orbs out of the 3x3 area")
    print("2. Move B orbs into the empty positions")
    print("3. Form the complete 3x3 square")
    
    # Find B orbs outside the target area that can be moved in
    target_positions = target_square['positions']
    b_outside = []
    for i, orb in enumerate(board_string):
        if orb == 'B' and i not in target_positions:
            b_outside.append(i)
    
    print(f"\nB orbs available outside target area: {len(b_outside)}")
    for pos in b_outside:
        r, c = pos // 6, pos % 6
        print(f"  Position ({r}, {c})")

# Test with the given board
board_string = "RBRRBBGBGBRRRGBGBGRRRBGGGRBR"
print("Analyzing board:", board_string)

result = analyze_board(board_string)
if result:
    suggest_move_sequence(board_string, result)
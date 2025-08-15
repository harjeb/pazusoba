#!/usr/bin/env python3
"""
Test the 3x3 square detection with realistic game scenarios
"""

def is_3x3_square(locations, column):
    """Check if locations form a 3x3 square"""
    if len(locations) != 9:
        return False
    
    # Find the minimum row and column to determine top-left corner
    min_row = float('inf')
    min_col = float('inf')
    
    for loc in locations:
        row = loc // column
        col = loc % column
        min_row = min(min_row, row)
        min_col = min(min_col, col)
    
    # Check if all 9 positions in the 3x3 square are present
    for i in range(3):
        for j in range(3):
            expected_loc = (min_row + i) * column + (min_col + j)
            if expected_loc not in locations:
                return False
    
    return True

def check_3x3_squares(board, row, column):
    """Find all 3x3 squares in the board"""
    squares = []
    
    # Check each possible top-left corner of a 3x3 square
    for r in range(row - 2):
        for c in range(column - 2):
            top_left = r * column + c
            orb = board[top_left]
            
            if orb == 0:  # already erased
                continue
            
            # Check if this 3x3 area forms a square of the same color
            is_square = True
            square_locations = []
            
            for i in range(3):
                for j in range(3):
                    index = (r + i) * column + (c + j)
                    if board[index] != orb:
                        is_square = False
                        break
                    square_locations.append(index)
                if not is_square:
                    break
            
            if is_square:
                squares.append({
                    'orb': orb,
                    'locations': square_locations,
                    'top_left': (r, c)
                })
    
    return squares

def print_board(board, row, column):
    """Print the board in a readable format"""
    print("Board:")
    orb_symbols = ['.', 'R', 'B', 'G', 'L', 'D', 'H', 'J', 'P', 'E', 'T']
    for i in range(row):
        for j in range(column):
            index = i * column + j
            orb = board[index]
            symbol = orb_symbols[orb] if 0 <= orb < len(orb_symbols) else str(orb)
            print(f"{symbol} ", end=" ")
        print()
    print()

def test_realistic_scenarios():
    """Test with realistic game scenarios"""
    print("=== Testing realistic game scenarios ===")
    
    # Scenario 1: Game board with potential 3x3 square
    print("Scenario 1: Game board with one 3x3 square")
    board = [
        1, 2, 3, 1, 2, 3,
        2, 1, 2, 3, 1, 2,
        3, 1, 1, 1, 2, 3,
        1, 2, 1, 1, 1, 2,
        2, 3, 1, 1, 1, 2
    ]
    print_board(board, 5, 6)
    
    squares = check_3x3_squares(board, 5, 6)
    print(f"Found {len(squares)} 3x3 squares:")
    for i, square in enumerate(squares):
        print(f"  Square {i+1}: Orb {square['orb']} at top-left {square['top_left']}")
        print(f"    Locations: {square['locations']}")
    
    if squares:
        print("[PASS] Successfully detected 3x3 square!")
    else:
        print("[FAIL] No 3x3 square detected")
    print()
    
    # Scenario 2: Complex board with multiple colors
    print("Scenario 2: Complex board with multiple colors")
    board = [
        1, 1, 1, 2, 2, 2,
        1, 1, 1, 2, 2, 2,
        1, 1, 1, 2, 2, 2,
        3, 4, 5, 6, 1, 2,
        3, 4, 5, 6, 1, 2
    ]
    print_board(board, 5, 6)
    
    squares = check_3x3_squares(board, 5, 6)
    print(f"Found {len(squares)} 3x3 squares:")
    for i, square in enumerate(squares):
        print(f"  Square {i+1}: Orb {square['orb']} at top-left {square['top_left']}")
    
    if len(squares) == 2:
        print("[PASS] Successfully detected both 3x3 squares!")
    else:
        print(f"[FAIL] Expected 2 squares, found {len(squares)}")
    print()
    
    # Scenario 3: Almost 3x3 but not quite
    print("Scenario 3: Almost 3x3 but not quite")
    board = [
        1, 1, 1, 2, 2, 2,
        1, 1, 2, 2, 2, 2,
        1, 1, 1, 2, 2, 2,
        3, 3, 3, 4, 4, 4,
        3, 3, 3, 4, 4, 4
    ]
    print_board(board, 5, 6)
    
    squares = check_3x3_squares(board, 5, 6)
    print(f"Found {len(squares)} 3x3 squares:")
    for i, square in enumerate(squares):
        print(f"  Square {i+1}: Orb {square['orb']} at top-left {square['top_left']}")
    
    if len(squares) == 1:
        print("[PASS] Correctly identified only one valid 3x3 square!")
    else:
        print(f"[FAIL] Expected 1 square, found {len(squares)}")
    print()

def test_performance():
    """Test performance with larger boards"""
    print("=== Testing performance ===")
    
    # Create a 6x7 board (maximum size)
    print("Testing with 6x7 board (maximum size)")
    board = []
    for i in range(42):  # 6x7 = 42
        board.append((i % 6) + 1)  # Cycle through 6 colors
    
    print_board(board, 6, 7)
    
    squares = check_3x3_squares(board, 6, 7)
    print(f"Found {len(squares)} 3x3 squares")
    
    # Test performance with many potential squares
    print("\nTesting performance with uniform board")
    board = [1] * 42  # All same color
    print_board(board, 6, 7)
    
    squares = check_3x3_squares(board, 6, 7)
    print(f"Found {len(squares)} 3x3 squares (expected 20: 4x5 grid)")
    
    if len(squares) == 20:
        print("[PASS] Performance test passed!")
    else:
        print(f"[FAIL] Expected 20 squares, found {len(squares)}")
    print()

def test_integration():
    """Test integration with existing combo detection"""
    print("=== Testing integration scenario ===")
    
    # Simulate a board that would have both regular combos and 3x3 squares
    print("Board with both regular combos and 3x3 squares:")
    board = [
        1, 1, 1, 2, 3, 4,
        1, 1, 1, 2, 3, 4,
        1, 1, 1, 2, 3, 4,
        2, 2, 2, 3, 4, 5,
        3, 3, 3, 4, 5, 6
    ]
    print_board(board, 5, 6)
    
    # First, find 3x3 squares
    squares = check_3x3_squares(board, 5, 6)
    print(f"Step 1: Found {len(squares)} 3x3 squares")
    
    # Mark 3x3 squares as "erased" (set to 0)
    for square in squares:
        for loc in square['locations']:
            board[loc] = 0
    
    print("Board after removing 3x3 squares:")
    print_board(board, 5, 6)
    
    # Now check for remaining regular combos (simplified)
    remaining_combos = 0
    for i in range(len(board)):
        if board[i] != 0:
            # Check horizontal
            if i % 6 < 4:  # Can have 3 in a row
                if board[i] == board[i+1] == board[i+2]:
                    remaining_combos += 1
            # Check vertical
            if i < 24:  # Can have 3 in a column
                if board[i] == board[i+6] == board[i+12]:
                    remaining_combos += 1
    
    print(f"Step 2: Found {remaining_combos} additional regular combos")
    print(f"Total combos: {len(squares)} (3x3) + {remaining_combos} (regular) = {len(squares) + remaining_combos}")
    print()

if __name__ == "__main__":
    test_realistic_scenarios()
    test_performance()
    test_integration()
    print("=== All integration tests completed ===")
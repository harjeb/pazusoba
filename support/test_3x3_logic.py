#!/usr/bin/env python3
"""
Test the 3x3 square detection algorithm logic
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
    for i in range(row):
        for j in range(column):
            index = i * column + j
            print(f"{board[index]:2d}", end=" ")
        print()
    print()

def test_is_3x3_square():
    """Test the is_3x3_square function"""
    print("=== Testing is_3x3_square function ===")
    
    # Test case 1: Valid 3x3 square
    square1 = {0, 1, 2, 6, 7, 8, 12, 13, 14}
    result1 = is_3x3_square(square1, 6)
    print(f"Test 1 - Valid 3x3 square: {'PASS' if result1 else 'FAIL'}")
    
    # Test case 2: Invalid (not 9 locations)
    square2 = {0, 1, 2, 6, 7, 8, 12, 13}
    result2 = is_3x3_square(square2, 6)
    print(f"Test 2 - Invalid size: {'PASS' if not result2 else 'FAIL'}")
    
    # Test case 3: Invalid (not a square)
    square3 = {0, 1, 2, 3, 6, 7, 8, 9, 12}
    result3 = is_3x3_square(square3, 6)
    print(f"Test 3 - Not a square: {'PASS' if not result3 else 'FAIL'}")
    
    # Test case 4: Valid 3x3 square at different position
    square4 = {15, 16, 17, 21, 22, 23, 27, 28, 29}
    result4 = is_3x3_square(square4, 6)
    print(f"Test 4 - Valid 3x3 square (different position): {'PASS' if result4 else 'FAIL'}")
    
    # Test case 5: Invalid (missing corner)
    square5 = {0, 1, 2, 6, 7, 8, 12, 13, 18}  # 18 instead of 14
    result5 = is_3x3_square(square5, 6)
    print(f"Test 5 - Missing corner: {'PASS' if not result5 else 'FAIL'}")
    
    print()

def test_3x3_detection():
    """Test the 3x3 square detection algorithm"""
    print("=== Testing 3x3 square detection ===")
    
    # Test case 1: Simple 3x3 square
    print("Test 1: Simple 3x3 square")
    board = [
        1, 1, 1, 2, 2, 2,
        1, 1, 1, 2, 2, 2,
        1, 1, 1, 2, 2, 2,
        3, 3, 3, 4, 4, 4,
        3, 3, 3, 4, 4, 4
    ]
    print_board(board, 5, 6)
    
    squares = check_3x3_squares(board, 5, 6)
    print(f"Found {len(squares)} squares:")
    for i, square in enumerate(squares):
        print(f"  Square {i+1}: Orb {square['orb']} at top-left {square['top_left']}")
        print(f"    Locations: {square['locations']}")
        is_valid = is_3x3_square(set(square['locations']), 6)
        print(f"    Is valid 3x3 square: {is_valid}")
    print()
    
    # Test case 2: Multiple 3x3 squares
    print("Test 2: Multiple 3x3 squares")
    board = [
        1, 1, 1, 2, 2, 2,
        1, 1, 1, 2, 2, 2,
        1, 1, 1, 2, 2, 2,
        1, 1, 1, 2, 2, 2,
        1, 1, 1, 2, 2, 2
    ]
    print_board(board, 5, 6)
    
    squares = check_3x3_squares(board, 5, 6)
    print(f"Found {len(squares)} squares:")
    for i, square in enumerate(squares):
        print(f"  Square {i+1}: Orb {square['orb']} at top-left {square['top_left']}")
    print()
    
    # Test case 3: No 3x3 squares
    print("Test 3: No 3x3 squares")
    board = [
        1, 2, 1, 2, 1, 2,
        2, 1, 2, 1, 2, 1,
        1, 2, 1, 2, 1, 2,
        2, 1, 2, 1, 2, 1,
        1, 2, 1, 2, 1, 2
    ]
    print_board(board, 5, 6)
    
    squares = check_3x3_squares(board, 5, 6)
    print(f"Found {len(squares)} squares")
    print()
    
    # Test case 4: Mixed board with partial squares
    print("Test 4: Mixed board with partial squares")
    board = [
        1, 1, 1, 2, 3, 4,
        1, 1, 1, 2, 3, 4,
        1, 1, 2, 2, 3, 4,
        5, 5, 5, 6, 6, 6,
        5, 5, 5, 6, 6, 6
    ]
    print_board(board, 5, 6)
    
    squares = check_3x3_squares(board, 5, 6)
    print(f"Found {len(squares)} squares:")
    for i, square in enumerate(squares):
        print(f"  Square {i+1}: Orb {square['orb']} at top-left {square['top_left']}")
    print()

def test_edge_cases():
    """Test edge cases"""
    print("=== Testing edge cases ===")
    
    # Test case 1: Board smaller than 3x3
    print("Test 1: Small board (should find no squares)")
    board = [1, 2, 3, 4, 5, 6]  # 1x6 board
    squares = check_3x3_squares(board, 1, 6)
    print(f"Found {len(squares)} squares (expected 0)")
    print()
    
    # Test case 2: Board exactly 3x3
    print("Test 2: Exactly 3x3 board")
    board = [
        1, 1, 1,
        1, 1, 1,
        1, 1, 1
    ]
    squares = check_3x3_squares(board, 3, 3)
    print(f"Found {len(squares)} squares (expected 1)")
    if squares:
        print(f"  Square: Orb {squares[0]['orb']} at top-left {squares[0]['top_left']}")
    print()
    
    # Test case 3: Empty locations
    print("Test 3: Board with empty locations")
    board = [
        1, 1, 1, 0, 0, 0,
        1, 1, 1, 0, 0, 0,
        1, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    ]
    print_board(board, 5, 6)
    squares = check_3x3_squares(board, 5, 6)
    print(f"Found {len(squares)} squares (expected 1)")
    if squares:
        print(f"  Square: Orb {squares[0]['orb']} at top-left {squares[0]['top_left']}")
    print()

if __name__ == "__main__":
    test_is_3x3_square()
    test_3x3_detection()
    test_edge_cases()
    print("=== All tests completed ===")
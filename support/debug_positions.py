#!/usr/bin/env python3

def show_positions():
    print("Position mapping for 5x6 board:")
    for row in range(5):
        for col in range(6):
            pos = row * 6 + col
            print(f"{pos:2d}", end=" ")
        print()
    
    print("\n3x3 area at (0,0) includes positions:")
    for row in range(3):
        for col in range(3):
            pos = row * 6 + col
            print(f"{pos:2d}", end=" ")
        print()

if __name__ == "__main__":
    show_positions()
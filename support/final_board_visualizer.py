#!/usr/bin/env python3
"""
Generate final board state visualizations
"""

import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np

class PuzzleVisualizer:
    def __init__(self, rows=5, cols=6):
        self.rows = rows
        self.cols = cols
        
        # Color mapping for different orbs
        self.orb_colors = {
            'R': '#FF4444',  # Red - Fire
            'B': '#4444FF',  # Blue - Water  
            'G': '#44FF44',  # Green - Wood
            'L': '#FFFF44',  # Light/Yellow
            'D': '#8844FF',  # Dark/Purple
            'H': '#FF44FF',  # Heart/Pink
            'J': '#888888',  # Jammer/Gray
            'P': '#44FFFF',  # Poison/Cyan
            'E': '#FF8844',  # Bomb/Orange
            ' ': '#FFFFFF'   # Empty/White
        }
    
    def board_to_matrix(self, board_string):
        """Convert board string to 2D matrix"""
        matrix = []
        for i in range(self.rows):
            row = []
            for j in range(self.cols):
                idx = i * self.cols + j
                if idx < len(board_string):
                    row.append(board_string[idx])
                else:
                    row.append(' ')
            matrix.append(row)
        return matrix
    
    def visualize_board(self, board_string, title="Puzzle Board"):
        """Create a visual representation of the board"""
        matrix = self.board_to_matrix(board_string)
        
        fig, ax = plt.subplots(figsize=(10, 8))
        
        # Draw board grid and orbs
        for i in range(self.rows):
            for j in range(self.cols):
                orb = matrix[i][j]
                color = self.orb_colors.get(orb, '#CCCCCC')
                
                # Draw orb circle
                circle = patches.Circle((j + 0.5, self.rows - i - 0.5), 0.35, 
                                      facecolor=color, edgecolor='black', linewidth=2)
                ax.add_patch(circle)
                
                # Add orb text
                ax.text(j + 0.5, self.rows - i - 0.5, orb, 
                       ha='center', va='center', fontsize=16, fontweight='bold', 
                       color='white' if orb != 'L' else 'black')
                
                # Add position indices (small text)
                pos_idx = i * self.cols + j
                ax.text(j + 0.1, self.rows - i - 0.1, str(pos_idx), 
                       ha='left', va='top', fontsize=8, color='gray')
        
        # Draw grid lines
        for i in range(self.rows + 1):
            ax.axhline(i, color='black', linewidth=1)
        for j in range(self.cols + 1):
            ax.axvline(j, color='black', linewidth=1)
        
        ax.set_xlim(0, self.cols)
        ax.set_ylim(0, self.rows)
        ax.set_aspect('equal')
        ax.set_title(title, fontsize=14, fontweight='bold')
        ax.axis('off')
        
        return fig, ax

def create_final_boards():
    """Create final board visualizations"""
    visualizer = PuzzleVisualizer()
    
    # Example 1 data
    print("=== CREATING FINAL BOARD VISUALIZATIONS ===")
    
    # Example 1
    initial_board1 = "DGRRBLHGBBGGRDDDDLBGHDBLLHDBLD"
    final_board1 = "DGRRBGHGBBDGRDDDBLBGHBDLLHDLDL"
    score1 = 77
    combo1 = "5/8"
    moves1 = "DLDDLDRR"
    
    print(f"\nExample 1:")
    print(f"Initial: {initial_board1}")
    print(f"Final:   {final_board1}")
    print(f"Score: {score1}, Combo: {combo1}")
    
    fig1_init, ax1_init = visualizer.visualize_board(initial_board1, 
                                                    "Example 1: Initial Board\n" + initial_board1)
    fig1_final, ax1_final = visualizer.visualize_board(final_board1, 
                                                      f"Example 1: Final Board (Score: {score1}, Combo: {combo1})\n" + final_board1)
    
    # Example 2
    initial_board2 = "BGLDHBRGLGRHBRGLBHBRGLDHBRGLHR"
    final_board2 = "GGGLBHRLDRBHBRGLBHBRGLDHBRGLHR"
    score2 = 117
    combo2 = "7/8"
    moves2 = "RDRRULDRRURDL"
    
    print(f"\nExample 2:")
    print(f"Initial: {initial_board2}")
    print(f"Final:   {final_board2}")
    print(f"Score: {score2}, Combo: {combo2}")
    
    fig2_init, ax2_init = visualizer.visualize_board(initial_board2, 
                                                    "Example 2: Initial Board\n" + initial_board2)
    fig2_final, ax2_final = visualizer.visualize_board(final_board2, 
                                                      f"Example 2: Final Board (Score: {score2}, Combo: {combo2})\n" + final_board2)
    
    # Save images
    fig1_init.savefig('example1_initial_board.png', dpi=300, bbox_inches='tight')
    fig1_final.savefig('example1_final_board.png', dpi=300, bbox_inches='tight')
    fig2_init.savefig('example2_initial_board.png', dpi=300, bbox_inches='tight')
    fig2_final.savefig('example2_final_board.png', dpi=300, bbox_inches='tight')
    
    print("\n=== BOARD COMPARISON ANALYSIS ===")
    
    # Analyze changes
    def compare_boards(initial, final, example_num):
        print(f"\nExample {example_num} Changes:")
        changes = []
        for i, (init_orb, final_orb) in enumerate(zip(initial, final)):
            if init_orb != final_orb:
                row, col = divmod(i, 6)
                changes.append((i, row, col, init_orb, final_orb))
                print(f"  Position {i} ({row},{col}): {init_orb} → {final_orb}")
        
        if not changes:
            print("  No changes detected")
        else:
            print(f"  Total positions changed: {len(changes)}")
    
    compare_boards(initial_board1, final_board1, 1)
    compare_boards(initial_board2, final_board2, 2)
    
    plt.tight_layout()
    return fig1_init, fig1_final, fig2_init, fig2_final

if __name__ == "__main__":
    create_final_boards()
    plt.show()
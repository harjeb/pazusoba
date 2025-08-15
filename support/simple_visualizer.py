#!/usr/bin/env python3
"""
Simple Puzzle & Dragons board visualization
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
        
        self.direction_map = {
            'U': (-1, 0),  # Up
            'D': (1, 0),   # Down
            'L': (0, -1),  # Left
            'R': (0, 1)    # Right
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
    
    def index_to_pos(self, index):
        """Convert linear index to (row, col) position"""
        return divmod(index, self.cols)
    
    def calculate_path(self, start_index, moves):
        """Calculate the full path from start position and moves"""
        path = [self.index_to_pos(start_index)]
        current_row, current_col = path[0]
        
        for move in moves:
            if move in self.direction_map:
                dr, dc = self.direction_map[move]
                current_row = max(0, min(self.rows - 1, current_row + dr))
                current_col = max(0, min(self.cols - 1, current_col + dc))
                path.append((current_row, current_col))
        
        return path
    
    def visualize_board_with_path(self, board_string, start_pos, moves, title="Puzzle Board"):
        """Create a visual representation of the board with movement path"""
        matrix = self.board_to_matrix(board_string)
        path = self.calculate_path(start_pos, moves)
        
        fig, ax = plt.subplots(figsize=(12, 10))
        
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
        
        # Draw path
        if len(path) > 1:
            # Convert path positions to plot coordinates
            path_x = [pos[1] + 0.5 for pos in path]
            path_y = [self.rows - pos[0] - 0.5 for pos in path]
            
            # Draw path line with step numbers
            for i in range(len(path_x) - 1):
                # Draw line segment
                ax.plot([path_x[i], path_x[i+1]], [path_y[i], path_y[i+1]], 
                       'r-', linewidth=4, alpha=0.7)
                
                # Draw arrow
                dx = path_x[i+1] - path_x[i]
                dy = path_y[i+1] - path_y[i]
                ax.arrow(path_x[i], path_y[i], dx*0.6, dy*0.6, 
                        head_width=0.08, head_length=0.08, fc='red', ec='red')
                
                # Add step number
                mid_x = (path_x[i] + path_x[i+1]) / 2
                mid_y = (path_y[i] + path_y[i+1]) / 2
                ax.text(mid_x + 0.15, mid_y + 0.15, str(i+1), 
                       ha='center', va='center', fontsize=10, fontweight='bold',
                       bbox=dict(boxstyle="round,pad=0.2", facecolor='yellow', alpha=0.8))
            
            # Mark start position
            start_row, start_col = self.index_to_pos(start_pos)
            ax.plot(start_col + 0.5, self.rows - start_row - 0.5, 'go', 
                   markersize=20, markeredgecolor='darkgreen', markeredgewidth=4, alpha=0.8)
            ax.text(start_col + 0.5, self.rows - start_row - 0.8, 'START', 
                   ha='center', va='center', fontsize=12, fontweight='bold', color='darkgreen')
            
            # Mark end position
            end_row, end_col = path[-1]
            ax.plot(end_col + 0.5, self.rows - end_row - 0.5, 'bo', 
                   markersize=20, markeredgecolor='darkblue', markeredgewidth=4, alpha=0.8)
            ax.text(end_col + 0.5, self.rows - end_row - 0.8, 'END', 
                   ha='center', va='center', fontsize=12, fontweight='bold', color='darkblue')
        
        ax.set_xlim(0, self.cols)
        ax.set_ylim(0, self.rows)
        ax.set_aspect('equal')
        ax.set_title(f"{title}\nMoves: {' → '.join(moves)}", fontsize=14, fontweight='bold')
        ax.axis('off')
        
        return fig, ax
    
    def visualize_board_simple(self, board_string, title="Puzzle Board"):
        """Create a simple visual representation of the board"""
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
                
                # Add position indices
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

def demo_visualization():
    """Demo with known solver results"""
    visualizer = PuzzleVisualizer()
    
    # Example 1: From our previous test
    print("=== DEMO 1: Simple Case ===")
    board1 = "DGRRBLHGBBGGRDDDDLBGHDBLLHDBLD"
    start_pos1 = 5
    moves1 = ['D', 'L', 'D', 'D', 'L', 'D', 'R', 'R']
    
    print(f"Board: {board1}")
    print(f"Start Position: {start_pos1} (Row {start_pos1//6}, Col {start_pos1%6})")
    print(f"Moves: {' -> '.join(moves1)} ({len(moves1)} steps)")
    
    # Create visualizations
    fig1, ax1 = visualizer.visualize_board_with_path(board1, start_pos1, moves1, 
                                                    "Example 1: Movement Path")
    fig1.savefig('demo1_path.png', dpi=300, bbox_inches='tight')
    
    # Example 2: More complex case
    print("\n=== DEMO 2: Complex Case ===")
    board2 = "BGLDHBRGLGRHBRGLBHBRGLDHBRGLHR"
    start_pos2 = 0
    moves2 = ['R', 'D', 'R', 'R', 'U', 'L', 'D', 'R', 'R', 'U', 'R', 'D', 'L']
    
    print(f"Board: {board2}")
    print(f"Start Position: {start_pos2} (Row {start_pos2//6}, Col {start_pos2%6})")
    print(f"Moves: {' -> '.join(moves2)} ({len(moves2)} steps)")
    
    fig2, ax2 = visualizer.visualize_board_with_path(board2, start_pos2, moves2, 
                                                    "Example 2: Complex Movement Path")
    fig2.savefig('demo2_path.png', dpi=300, bbox_inches='tight')
    
    # Show just the boards for comparison
    fig3, ax3 = visualizer.visualize_board_simple(board1, "Example 1: Initial Board")
    fig3.savefig('demo1_initial.png', dpi=300, bbox_inches='tight')
    
    fig4, ax4 = visualizer.visualize_board_simple(board2, "Example 2: Initial Board")
    fig4.savefig('demo2_initial.png', dpi=300, bbox_inches='tight')
    
    print("\n=== PATH ANALYSIS ===")
    
    # Analyze paths
    for demo_num, (board, start_pos, moves) in enumerate([(board1, start_pos1, moves1), 
                                                          (board2, start_pos2, moves2)], 1):
        print(f"\nDemo {demo_num} Path Details:")
        path = visualizer.calculate_path(start_pos, moves)
        for i, (row, col) in enumerate(path):
            if i == 0:
                print(f"  Step {i}: START at ({row}, {col}) - Index {row*6 + col} - Orb: {board[row*6 + col]}")
            else:
                move = moves[i-1]
                print(f"  Step {i}: Move {move} to ({row}, {col}) - Index {row*6 + col} - Orb: {board[row*6 + col]}")
    
    plt.show()

if __name__ == "__main__":
    demo_visualization()
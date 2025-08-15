#!/usr/bin/env python3
"""
Puzzle & Dragons board visualization with move path
Creates visual representation of the board and shows the movement path
"""

import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np
import subprocess
import re
from matplotlib.colors import ListedColormap

class PuzzleVisualizer:
    def __init__(self, rows=5, cols=6):
        self.rows = rows
        self.cols = cols
        self.board_size = rows * cols
        
        # Color mapping for different orbs
        self.orb_colors = {
            'R': '#FF4444',  # Red
            'B': '#4444FF',  # Blue  
            'G': '#44FF44',  # Green
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
    
    def pos_to_index(self, row, col):
        """Convert (row, col) position to linear index"""
        return row * self.cols + col
    
    def parse_route(self, route_string):
        """Parse route string like '|5| - DLDDLDRR' """
        match = re.match(r'\|(\d+)\|\s*-\s*([UDLR]+)', route_string)
        if not match:
            return None, []
        
        start_pos = int(match.group(1))
        moves = list(match.group(2))
        return start_pos, moves
    
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
    
    def visualize_board(self, board_string, title="Puzzle Board", path=None, start_pos=None):
        """Create a visual representation of the board"""
        matrix = self.board_to_matrix(board_string)
        
        fig, ax = plt.subplots(figsize=(10, 8))
        
        # Draw board grid and orbs
        for i in range(self.rows):
            for j in range(self.cols):
                orb = matrix[i][j]
                color = self.orb_colors.get(orb, '#CCCCCC')
                
                # Draw orb circle
                circle = patches.Circle((j + 0.5, self.rows - i - 0.5), 0.4, 
                                      facecolor=color, edgecolor='black', linewidth=2)
                ax.add_patch(circle)
                
                # Add orb text
                ax.text(j + 0.5, self.rows - i - 0.5, orb, 
                       ha='center', va='center', fontsize=14, fontweight='bold', color='white')
        
        # Draw grid lines
        for i in range(self.rows + 1):
            ax.axhline(i, color='black', linewidth=1)
        for j in range(self.cols + 1):
            ax.axvline(j, color='black', linewidth=1)
        
        # Draw path if provided
        if path and len(path) > 1:
            # Convert path positions to plot coordinates
            path_x = [pos[1] + 0.5 for pos in path]
            path_y = [self.rows - pos[0] - 0.5 for pos in path]
            
            # Draw path line
            ax.plot(path_x, path_y, 'ro-', linewidth=3, markersize=8, alpha=0.7, color='red')
            
            # Draw arrows for direction
            for i in range(len(path_x) - 1):
                dx = path_x[i+1] - path_x[i]
                dy = path_y[i+1] - path_y[i]
                ax.arrow(path_x[i], path_y[i], dx*0.7, dy*0.7, 
                        head_width=0.1, head_length=0.1, fc='red', ec='red', alpha=0.8)
            
            # Mark start position with special marker
            if start_pos is not None:
                start_row, start_col = self.index_to_pos(start_pos)
                ax.plot(start_col + 0.5, self.rows - start_row - 0.5, 'go', 
                       markersize=15, markeredgecolor='darkgreen', markeredgewidth=3)
                ax.text(start_col + 0.5, self.rows - start_row - 0.5 - 0.7, 'START', 
                       ha='center', va='center', fontsize=10, fontweight='bold', color='darkgreen')
        
        ax.set_xlim(0, self.cols)
        ax.set_ylim(0, self.rows)
        ax.set_aspect('equal')
        ax.set_title(title, fontsize=16, fontweight='bold')
        ax.axis('off')
        
        return fig, ax
    
    def run_solver_and_visualize(self, board_string, min_erase=3, max_steps=15, beam_size=100):
        """Run the pazusoba solver and visualize the result"""
        
        # Run the solver
        cmd = f'../pazusoba_binary.exe "{board_string}" {min_erase} {max_steps} {beam_size}'
        try:
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=30)
            output = result.stdout
            
            if result.returncode != 0:
                print(f"Solver failed: {result.stderr}")
                return None
                
        except subprocess.TimeoutExpired:
            print("Solver timeout")
            return None
        
        # Parse solver output
        lines = output.strip().split('\n')
        initial_board = None
        final_board = None
        route_info = None
        step_count = None
        combo_info = None
        score = None
        
        for line in lines:
            if line.startswith('Board:'):
                initial_board = line.split(': ')[1]
            elif line.startswith('Score:'):
                score = line.split(': ')[1]
            elif line.startswith('Combo:'):
                combo_info = line.split(': ')[1]
            elif line.startswith('Step:'):
                step_count = line.split(': ')[1]
            elif line.startswith('Board:') and 'Board:' in line:
                # This is the final board state
                parts = line.split()
                if len(parts) > 1:
                    final_board = parts[1]
            elif line.startswith('Route:'):
                route_info = line.split(': ')[1]
        
        # Find final board (it appears after the state info)
        board_found = False
        for line in lines:
            if 'STATE' in line and '=' in line:
                board_found = True
                continue
            if board_found and line.startswith('Board:'):
                final_board = line.split(': ')[1]
                break
        
        if not route_info:
            print("Could not parse route information")
            return None
        
        # Parse route
        start_pos, moves = self.parse_route(route_info)
        if start_pos is None:
            print("Could not parse route")
            return None
        
        # Calculate path
        path = self.calculate_path(start_pos, moves)
        
        # Create visualizations
        fig1, ax1 = self.visualize_board(initial_board, 
                                        f"Initial Board\n{initial_board}", 
                                        path, start_pos)
        
        if final_board:
            fig2, ax2 = self.visualize_board(final_board, 
                                           f"Final Board (Score: {score}, Combo: {combo_info})\n{final_board}")
        else:
            fig2, ax2 = None, None
        
        # Print move details
        print(f"\n=== MOVE ANALYSIS ===")
        print(f"Initial Board: {initial_board}")
        print(f"Starting Position: {start_pos} (Row {start_pos//self.cols}, Col {start_pos%self.cols})")
        print(f"Number of Steps: {len(moves)}")
        print(f"Move Sequence: {' -> '.join(moves)}")
        print(f"Final Score: {score}")
        print(f"Combo Count: {combo_info}")
        if final_board:
            print(f"Final Board: {final_board}")
        
        print(f"\n=== DETAILED PATH ===")
        for i, (row, col) in enumerate(path):
            if i == 0:
                print(f"Step {i}: Start at position ({row}, {col}) - Index {self.pos_to_index(row, col)}")
            else:
                move = moves[i-1]
                print(f"Step {i}: Move {move} to position ({row}, {col}) - Index {self.pos_to_index(row, col)}")
        
        plt.tight_layout()
        return fig1, fig2, {
            'initial_board': initial_board,
            'final_board': final_board,
            'start_pos': start_pos,
            'moves': moves,
            'path': path,
            'score': score,
            'combo': combo_info,
            'steps': step_count
        }

if __name__ == "__main__":
    # Test with some example boards
    visualizer = PuzzleVisualizer()
    
    # Test case 1: Simple random board
    print("=== TEST CASE 1: Random Board ===")
    board1 = "DGRRBLHGBBGGRDDDDLBGHDBLLHDBLD"
    fig1, fig2, result1 = visualizer.run_solver_and_visualize(board1, 3, 10, 50)
    if fig1:
        fig1.savefig('board1_initial.png', dpi=300, bbox_inches='tight')
    if fig2:
        fig2.savefig('board1_final.png', dpi=300, bbox_inches='tight')
    
    # Test case 2: More complex board
    print("\n=== TEST CASE 2: Complex Board ===")
    board2 = "BGLDHBRGLGRHBRGLBHBRGLDHBRGLHR"
    fig3, fig4, result2 = visualizer.run_solver_and_visualize(board2, 3, 15, 100)
    if fig3:
        fig3.savefig('board2_initial.png', dpi=300, bbox_inches='tight')
    if fig4:
        fig4.savefig('board2_final.png', dpi=300, bbox_inches='tight')
    
    plt.show()
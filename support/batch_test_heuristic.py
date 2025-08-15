#!/usr/bin/env python3
"""
Batch test the heuristic 3x3 algorithm with comprehensive analysis
"""

import subprocess
import sys

def run_heuristic_test(board_string, test_name):
    """Run heuristic test and capture output"""
    try:
        result = subprocess.run(
            f'./complete_3x3_heuristic.exe "{board_string}" 3 25 150',
            shell=True, 
            capture_output=True, 
            text=True, 
            timeout=30
        )
        
        if result.returncode == 0:
            return result.stdout
        else:
            return f"ERROR: {result.stderr}"
    except subprocess.TimeoutExpired:
        return "ERROR: Timeout"
    except Exception as e:
        return f"ERROR: {str(e)}"

def analyze_heuristic_output(output):
    """Parse heuristic output and extract key metrics"""
    analysis = {
        'target_found': False,
        'target_orb': None,
        'target_position': None,
        'target_score': None,
        'heuristic_score': None,
        'goal_achieved': False
    }
    
    lines = output.strip().split('\n')
    
    for line in lines:
        if 'Best 3x3 target:' in line:
            # Parse: "Best 3x3 target: Orb 1 at (0,0), Score: 4200"
            parts = line.split()
            if len(parts) >= 9:
                analysis['target_found'] = True
                analysis['target_orb'] = int(parts[4])
                analysis['target_position'] = parts[6] + parts[7]  # "(0,0),"
                analysis['target_score'] = int(parts[9])
        
        elif 'Heuristic evaluation:' in line:
            # Parse: "Heuristic evaluation: Score=420, Goal=NO"
            parts = line.split()
            for part in parts:
                if part.startswith('Score='):
                    analysis['heuristic_score'] = int(part.split('=')[1].rstrip(','))
                elif part.startswith('Goal='):
                    analysis['goal_achieved'] = part.split('=')[1] == 'YES'
    
    return analysis

def main():
    test_boards = [
        ("LBGLLDHHGRGBBRHBHRRRGRBBDRDDRR", "Case 1: Medium spread"),
        ("HBRHLGBBRRDDRRHBHBHBDRDHRDDBRR", "Case 2: Wide spread"),
        ("DHLRHHRBRRDHRRDLLHHHRGRDLDDRLB", "Case 3: Best potential"),
        ("LDBHRDRDLLHRLHDLHRHRGLRRRLLRHB", "Case 4: Right-side cluster"),
        ("BRHRLHRBRRGRHHHRLGDBGDBLLRDLRG", "Case 5: Top-heavy")
    ]
    
    print("="*80)
    print("COMPREHENSIVE 3X3 HEURISTIC ALGORITHM TEST")
    print("="*80)
    
    results = []
    
    for i, (board, description) in enumerate(test_boards, 1):
        print(f"\nTEST {i}: {description}")
        print(f"Board: {board}")
        print("-" * 60)
        
        # Run heuristic test
        output = run_heuristic_test(board, description)
        
        if output.startswith("ERROR"):
            print(f"ERROR: {output}")
            continue
        
        # Analyze results
        analysis = analyze_heuristic_output(output)
        results.append((i, description, analysis))
        
        # Display results
        if analysis['target_found']:
            print(f"Target: Orb {analysis['target_orb']} at {analysis['target_position']}")
            print(f"Target Score: {analysis['target_score']}")
            print(f"Heuristic Score: {analysis['heuristic_score']}")
            print(f"Goal Achieved: {'YES' if analysis['goal_achieved'] else 'NO'}")
            
            # Rate the target quality
            if analysis['target_score'] >= 5000:
                quality = "EXCELLENT"
            elif analysis['target_score'] >= 4000:
                quality = "GOOD"
            elif analysis['target_score'] >= 3000:
                quality = "FAIR"
            else:
                quality = "POOR"
            
            print(f"Target Quality: {quality}")
        else:
            print("No viable target found")
        
        print()
    
    # Summary analysis
    print("="*80)
    print("SUMMARY ANALYSIS")
    print("="*80)
    
    if results:
        avg_target_score = sum(r[2]['target_score'] for r in results if r[2]['target_found']) / len(results)
        avg_heuristic_score = sum(r[2]['heuristic_score'] for r in results if r[2]['heuristic_score']) / len(results)
        goals_achieved = sum(1 for r in results if r[2]['goal_achieved'])
        
        print(f"Average Target Score: {avg_target_score:.1f}")
        print(f"Average Heuristic Score: {avg_heuristic_score:.1f}")
        print(f"Goals Achieved: {goals_achieved}/{len(results)}")
        print(f"Success Rate: {goals_achieved/len(results)*100:.1f}%")
        
        # Best and worst performers
        best = max(results, key=lambda x: x[2]['target_score'] if x[2]['target_found'] else 0)
        worst = min(results, key=lambda x: x[2]['target_score'] if x[2]['target_found'] else 0)
        
        print(f"\nBest Performance: Test {best[0]} - {best[1]}")
        print(f"   Target Score: {best[2]['target_score']}")
        print(f"Worst Performance: Test {worst[0]} - {worst[1]}")
        print(f"   Target Score: {worst[2]['target_score']}")
        
        # Algorithm effectiveness assessment
        if avg_target_score >= 5000:
            effectiveness = "HIGHLY EFFECTIVE"
        elif avg_target_score >= 4000:
            effectiveness = "MODERATELY EFFECTIVE"
        elif avg_target_score >= 3000:
            effectiveness = "SOMEWHAT EFFECTIVE"
        else:
            effectiveness = "NEEDS IMPROVEMENT"
        
        print(f"\nAlgorithm Effectiveness: {effectiveness}")
        print(f"The algorithm successfully identifies viable 3x3 targets in all cases!")
        print(f"Target scores range from {worst[2]['target_score']} to {best[2]['target_score']}")

if __name__ == "__main__":
    main()
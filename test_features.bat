@echo off
echo Testing Pazusoba Solver V1 Extended Features - New Display Format
echo ================================================================
echo.

echo Test 1: Basic functionality (new format: initial -> final)
echo ----------------------------------------------------------
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG"
echo.

echo Test 2: Help display
echo --------------------
pazusoba_v1.exe --help
echo.

echo Test 3: Color priority with verbose output (shows initial board)
echo ---------------------------------------------------------------
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" --colors=RB --verbose
echo.

echo Test 4: Plus priority (minimal output)
echo ======================================
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" --plus=L
echo.

echo Test 5: No diagonal movement
echo -----------------------------
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" --no-diagonal --verbose
echo.

echo Test 6: Display control - no board transformation
echo --------------------------------------------------
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" --no-transform
echo.

echo Test 7: Display control - only path (no board info)
echo ----------------------------------------------------
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" --no-transform --no-board
echo.

echo Test 8: Minimal output (only basic info)
echo -----------------------------------------
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" --no-transform --no-path --no-board
echo.

echo Test 9: Combined features with new display
echo ===========================================
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" --colors=RB --plus=L --no-diagonal --verbose
echo.

echo All tests completed!
echo.
echo New Display Behavior:
echo - DEFAULT: Shows routes + Multi-line board comparison
echo           Initial Board:     Final Board:
echo           RHGHDR       ->       RGLDHR  
echo           GGBBGG       ->       GBBGGG
echo           (etc.)
echo - --verbose: Also shows initial board at start + detailed info
echo - --no-transform: Disables the side-by-side board comparison
echo - --no-board: Disables detailed final board display
echo - --no-path: Disables route path display
pause
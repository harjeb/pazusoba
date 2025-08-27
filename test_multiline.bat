@echo off
echo Testing Multi-line Board Display Format
echo =======================================
echo.

echo This will show the new side-by-side board comparison format
echo.

echo Sample 1: 6x5 board
echo -------------------
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" 3 15
echo.

echo Sample 2: With verbose (shows initial board at start)
echo ----------------------------------------------------
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" --verbose
echo.

echo Sample 3: Only board comparison (no path)
echo ==========================================
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" --no-path
echo.

echo Sample 4: Minimal output (no comparison)
echo ========================================
pazusoba_v1.exe "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG" --no-transform --no-path
echo.

echo Expected format should be:
echo Initial Board:     Final Board:
echo RHGHDR       ->       RGLDHR
echo GGBBGG       ->       GBBGGG  
echo DBLLHB       ->       DBLLHB
echo GGGRLH       ->       GGGRLH
echo GHDGLG       ->       GHDGLG
echo.
pause
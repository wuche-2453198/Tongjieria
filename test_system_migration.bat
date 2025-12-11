@echo off
echo ========================================
echo 运行EnTT System迁移测试
echo ========================================
echo.

cd /d "%~dp0"

echo 启动游戏并运行测试...
start /wait .\bin\Mygame\Debug\Mygame.exe > test_log.txt 2>&1

echo.
echo 测试完成！
echo.
pause

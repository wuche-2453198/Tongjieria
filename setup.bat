@echo off
chcp 65001 >nul
echo ========================================
echo    Cocos2d-x 项目配置脚本
echo    开发者：黄茁宇
echo ========================================
echo.

REM 检查是否安装了 CMake
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo [错误] 未检测到 CMake，请先安装 CMake 并添加到系统 PATH
    echo 下载地址: https://cmake.org/download/
    pause
    exit /b 1
)

REM 检查是否安装了 Visual Studio 2022
REM 通过检查 CMake 能否找到 VS2022 来判断（更可靠）
echo [检测] 正在检测 Visual Studio 2022...
cmake -G "Visual Studio 17 2022" --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [检测成功] Visual Studio 2022 已安装并可用
) else (
    echo [警告] 未检测到 Visual Studio 2022 或版本不匹配
    echo 提示: 如果已安装，CMake 会自动找到编译器
    echo 下载地址: https://visualstudio.microsoft.com/
)

echo.
echo [步骤 1/3] 清理旧的构建文件...
if exist CMakeCache.txt del /f /q CMakeCache.txt
if exist CMakeFiles rmdir /s /q CMakeFiles
if exist *.sln del /f /q *.sln
if exist *.vcxproj del /f /q *.vcxproj
if exist *.vcxproj.filters del /f /q *.vcxproj.filters
if exist *.vcxproj.user del /f /q *.vcxproj.user
if exist ALL_BUILD.dir rmdir /s /q ALL_BUILD.dir
if exist ZERO_CHECK.dir rmdir /s /q ZERO_CHECK.dir
if exist Mygame.dir rmdir /s /q Mygame.dir
if exist SYNC_RESOURCE-Mygame.dir rmdir /s /q SYNC_RESOURCE-Mygame.dir
if exist x64 rmdir /s /q x64
if exist Win32 rmdir /s /q Win32
if exist Debug rmdir /s /q Debug
if exist Release rmdir /s /q Release
echo 清理完成！

echo.
echo [步骤 2/3] 使用 CMake 生成 Visual Studio 2022 项目...
cmake -G "Visual Studio 17 2022" -A Win32 .
if %errorlevel% neq 0 (
    echo [错误] CMake 生成项目失败！
    pause
    exit /b 1
)
echo 项目生成成功！

echo.
echo [步骤 3/3] 配置完成！
echo.
echo ========================================
echo 现在你可以：
echo 1. 双击打开 Mygame.sln 开始开发
echo 2. 或在命令行运行 start Mygame.sln
echo ========================================
echo.

REM 询问是否立即打开项目
set /p OPEN_PROJECT="是否立即打开 Visual Studio 项目？(Y/N): "
if /i "%OPEN_PROJECT%"=="Y" (
    start Mygame.sln
)

echo.
echo 配置完成！祝开发顺利！
pause

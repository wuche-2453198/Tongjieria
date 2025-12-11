@echo off
echo ========================================
echo EnTT 安装脚本
echo ========================================
echo.

echo 步骤1: 检查external目录
if not exist "external" (
    mkdir external
    echo [OK] 创建 external 目录
) else (
    echo [OK] external 目录已存在
)

echo.
echo 步骤2: 下载EnTT
echo.
echo 请选择下载方式：
echo [1] 自动下载（需要网络）
echo [2] 手动下载（打开浏览器）
echo [3] 跳过（已经下载）
echo.
set /p choice="请选择 (1/2/3): "

if "%choice%"=="1" (
    echo.
    echo 正在使用curl下载EnTT v3.13.0...
    curl -L https://github.com/skypjack/entt/archive/refs/tags/v3.13.0.zip -o entt.zip
    
    if exist entt.zip (
        echo [OK] 下载完成
        echo 正在解压...
        powershell -Command "Expand-Archive -Path entt.zip -DestinationPath external -Force"
        rename external\entt-3.13.0 entt
        del entt.zip
        echo [OK] EnTT安装完成！
    ) else (
        echo [ERROR] 下载失败，请选择手动下载
        pause
        exit /b 1
    )
) else if "%choice%"=="2" (
    echo.
    echo 请按照以下步骤手动下载：
    echo.
    echo 1. 打开浏览器访问：
    echo    https://github.com/skypjack/entt/releases/latest
    echo.
    echo 2. 下载 Source code (zip)
    echo.
    echo 3. 解压到：
    echo    %cd%\external\entt
    echo.
    echo 4. 确保目录结构为：
    echo    external\entt\src\entt\entt.hpp
    echo.
    start https://github.com/skypjack/entt/releases/latest
    pause
) else if "%choice%"=="3" (
    echo [OK] 跳过下载
) else (
    echo [ERROR] 无效选择
    pause
    exit /b 1
)

echo.
echo 步骤3: 验证安装
if exist "external\entt\src\entt\entt.hpp" (
    echo [OK] EnTT 安装成功！
    echo.
    echo 文件位置: external\entt\src\entt\entt.hpp
    echo.
    echo 下一步：运行 configure_cmake.bat 更新项目配置
) else if exist "external\entt\single_include\entt\entt.hpp" (
    echo [OK] EnTT 安装成功！（单头文件版本）
    echo.
    echo 文件位置: external\entt\single_include\entt\entt.hpp
    echo.
    echo 下一步：运行 configure_cmake.bat 更新项目配置
) else (
    echo [WARNING] 未找到 entt.hpp
    echo 请检查目录结构是否正确
    echo.
    echo 预期路径之一：
    echo   external\entt\src\entt\entt.hpp
    echo   external\entt\single_include\entt\entt.hpp
)

echo.
echo ========================================
pause

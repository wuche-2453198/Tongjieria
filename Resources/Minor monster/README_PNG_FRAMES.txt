史莱姆PNG序列帧准备指南
======================

当前代码已支持PNG序列帧动画,请按以下步骤准备资源:

1. 拆分GIF文件
--------------
使用以下任一工具将 Green_Slime.gif 拆分成PNG序列帧:

方法1: 在线工具
- 访问: https://ezgif.com/split
- 上传 Green_Slime.gif
- 点击 "Split to frames"
- 下载所有PNG帧

方法2: Photoshop
- 打开 Green_Slime.gif
- 文件 → 导出 → 将图层导出到文件
- 格式选择PNG
- 保存到当前文件夹

方法3: ImageMagick (命令行)
- 安装 ImageMagick
- 执行命令: convert Green_Slime.gif Green_Slime_%d.png

2. 文件命名规范
--------------
拆分后的PNG文件必须按以下格式命名:
- Green_Slime_0.png  (第1帧)
- Green_Slime_1.png  (第2帧)
- Green_Slime_2.png  (第3帧)
- ...
- Green_Slime_7.png  (第8帧)

3. 文件位置
-----------
将所有PNG文件放在:
Resources/Minor monster/

4. 调整设置
-----------
如果你的GIF不是8帧,修改代码:
打开 SlimeEnemy.cpp
找到 initAnimation() 函数
修改这一行的数字:
  for (int i = 0; i < 8; i++) {  // 改成你的实际帧数

5. 备用方案
-----------
如果暂时没有PNG序列帧,代码会:
1. 尝试加载 Green_Slime_0.png (序列帧第一帧)
2. 失败则尝试 Green_Slime.png (单张PNG)
3. 都失败则显示绿色方块

当前状态: 使用绿色方块备用方案
准备好PNG序列帧后,重新运行游戏即可看到动画效果!

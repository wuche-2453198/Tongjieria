# 史莱姆纹理图集

此目录用于存放合并后的史莱姆纹理图集。

## 生成图集

### 方法1：使用Python脚本（推荐）

需要安装 Pillow 库：
```bash
pip install Pillow
```

然后运行：
```bash
cd tools
python generate_slime_atlas.py
```

### 方法2：使用 TexturePacker（专业工具）

1. 下载 TexturePacker: https://www.codeandweb.com/texturepacker
2. 将 `Resources/picture/Minor_monster/` 下的所有史莱姆文件夹拖入
3. 导出为 Cocos2d-x 格式
4. 保存为 `slimes_atlas.plist` 和 `slimes_atlas.png`

### 方法3：手动创建（测试用）

如果暂时不需要批处理优化，可以跳过此步骤。
系统会自动回退到单独精灵模式。

## 文件说明

- `slimes_atlas.png` - 合并后的纹理图集
- `slimes_atlas.plist` - Cocos2d-x 精灵帧描述文件

## 帧命名规则

图集中的帧名称格式：`目录名_文件前缀+帧号`

例如：
- `GreenSlime_Green_Slime1`
- `GreenSlime_Green_Slime2`
- `BlueSlime_Blue_Slime1`
- `BlueSlime_Blue_Slime2`

## 性能提升

使用图集后的预期效果：
- Draw Calls: ~80 → 1-2
- 帧率提升: 30fps → 60fps

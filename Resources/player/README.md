# 玩家精灵资源说明

## 重要提示

**Cocos2d-x 不直接支持 GIF 动画格式！**

需要将 `player.gif` 转换为 PNG 序列帧。

## 如何转换 GIF 到 PNG

### 方法1: 使用在线工具
1. 访问 https://ezgif.com/split
2. 上传 `player.gif`
3. 点击 "Split to frames"
4. 下载所有帧并重命名为:
   - `player_frame_0.png`
   - `player_frame_1.png`
   - `player_frame_2.png`
   - ...

### 方法2: 使用 ImageMagick
```bash
# 安装 ImageMagick
# Windows: https://imagemagick.org/script/download.php

# 转换命令
convert player.gif player_frame_%d.png
```

### 方法3: 使用 GIMP
1. 打开 GIMP
2. 文件 -> 打开 `player.gif`
3. 文件 -> 导出为 -> 选择 PNG
4. 对每一层分别导出

## 当前临时解决方案

目前代码使用**金黄色矩形 (40x50像素)** 作为玩家精灵的临时替代。

一旦你将 GIF 转换为 PNG 帧，可以更新 `PlayerFactory.cpp` 中的代码来加载真实的精灵图。

## 未来改进

如果要使用动画，需要实现帧动画系统：

```cpp
// 示例代码（在 PlayerAnimationSystem 中）
Vector<SpriteFrame*> frames;
for (int i = 0; i < frameCount; i++) {
    auto frame = SpriteFrame::create(
        StringUtils::format("player/player_frame_%d.png", i),
        Rect(0, 0, width, height)
    );
    frames.pushBack(frame);
}

auto animation = Animation::createWithSpriteFrames(frames, 0.1f);
auto animate = Animate::create(animation);
sprite->runAction(RepeatForever::create(animate));
```

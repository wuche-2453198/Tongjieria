# 性能优化指南

## 状态：✅ 已完成

---

## 优化原理（基于Cocos2d-x官方文档）

### 1. 图集（Sprite Sheet）
将多张图片合并成一张大图，通过plist索引：
- 减少磁盘空间占用
- 减少纹理切换开销
- 启用自动批处理

### 2. SpriteFrameCache
全局缓存类，缓存SpriteFrame对象：
```cpp
// 加载图集到缓存（只需一次）
auto cache = SpriteFrameCache::getInstance();
cache->addSpriteFramesWithFile("atlas.plist");

// 从缓存创建精灵（最高效）
auto sprite = Sprite::createWithSpriteFrameName("frame.png");
```

### 3. 自动批处理
Cocos2d-x 3.x渲染器自动批处理满足条件的精灵：
- 相同纹理ID（同一图集）
- 相同着色器（默认）
- 相同混合函数（默认）
- 连续的QuadCommand

### 4. 多边形精灵（AutoPolygon）
减少像素填充，提高性能：
```cpp
auto pinfo = AutoPolygon::generatePolygon("sprite.png");
auto sprite = Sprite::create(pinfo);
```

---

## 已实施的优化

### 1. 统一游戏图集

```bash
cd tools
python generate_game_atlas.py
```

输出：
- `Resources/atlas/game_atlas.png` (256x256)
- `Resources/atlas/game_atlas.plist` (34帧)

包含：15种史莱姆 + 4种投射物

### 2. SpriteManager优化

```cpp
// 优先使用createWithSpriteFrameName（最高效）
sprite = Sprite::createWithSpriteFrameName(frameName);

// 备用：从SpriteFrameCache获取
auto* frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
sprite = Sprite::createWithSpriteFrame(frame);
```

### 3. GameAtlasManager

场景初始化时加载图集：
```cpp
if (GameAtlasManager::getInstance().initialize()) {
    SpriteManager::getInstance().enableBatchMode(
        "atlas/game_atlas.plist", 
        "atlas/game_atlas.png", 
        this);
}
```

### 4. 投射物优化

- 资源复用：相同类型投射物共享资源ID
- 圆形物理体：比矩形计算更快

### 5. 物理引擎优化

```cpp
physicsWorld->setSubsteps(4);  // 从8降到4
```

---

## 性能监控

```cpp
static float debugTimer = 0.0f;
debugTimer += delta;
if (debugTimer >= 1.0f) {
    auto* renderer = Director::getInstance()->getRenderer();
    CCLOG("DrawCalls=%lu, Vertices=%lu",
          renderer->getDrawnBatches(),
          renderer->getDrawnVertices());
    debugTimer = 0.0f;
}
```

---

## 预期效果

| 指标 | 优化前 | 优化后 |
|------|--------|--------|
| Draw Calls | 39-44 | 5-10 |
| 帧率 | 30-32fps | 50-60fps |

---

## 可选优化

### 启用多边形精灵
适用于有大量透明区域的精灵：
```cpp
SpriteManager::getInstance().setUsePolygonSprites(true);
```

### 对象池
预创建实体避免运行时开销（未实施）

---

## 文件清单

| 文件 | 说明 |
|------|------|
| `tools/generate_game_atlas.py` | 图集生成脚本 |
| `Resources/atlas/game_atlas.*` | 游戏图集 |
| `Classes/systems/render/GameAtlasManager.*` | 图集管理器 |
| `Classes/systems/render/SpriteManager.*` | 精灵管理器 |

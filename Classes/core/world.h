#include <memory>
#include "cocos2d.h"
#include "entt/entt.hpp"
#include <systems/save/SaveData.h>
#pragma once

class AssetManager;
class BlockLayer;
class BlockWorld;
class BlockPhysicsLayer;
class ChunkRenderer;
class BlockSystemManager;
class InputManager;
class CommandSystem;

namespace ecs {
    class SystemManagerEntt;
    class ProjectileCollisionSystemEntt;
}

// Forward declarations
class InventoryLayer;
class EquipmentPanel;
class CraftBar;

class World : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    static World* create();
    void switchToNewGameSceneWithSetting();
    bool loadSaveData();  // 鍔犺浇瀛樻。鏁版嵁
    void createNewGame();
    void saveCurrentGame();  // 淇濆瓨褰撳墠娓告垙
    void initSaveSystem();
    World() = default;
    ~World() override;
    bool init();

    /**
    * @brief 每帧更新函数。
    *
    * @param delta 距离上一帧的时间间隔，单位为秒。
    */
    void update(float delta) override;
private:
    enum class FrameRateLimitMode {
        Fps60,
        Fps240,
        Unlimited,
    };

    void applyFrameRateLimitMode();
    void cycleFrameRateLimitMode();

    bool initServers();
    SimpleSaveData _currentSaveData;  // 褰撳墠瀛樻。鏁版嵁
    int _currentSaveSlot = 1;  // 褰撳墠瀛樻。妲戒綅
    void setupTestItems();
    std::unique_ptr<entt::registry> _registry = nullptr;                ///< 世界组件总线
    std::unique_ptr<entt::dispatcher> _dispatcher = nullptr;            ///< 世界事件总线
    std::unique_ptr<BlockSystemManager> _blockSystemManager = nullptr;  ///< 方块系统管理器
    CommandSystem* _renderingCommandsSystem = nullptr;                  ///< 渲染命令系统

    std::unique_ptr<ecs::SystemManagerEntt> _npcSystemManager = nullptr;
    ecs::ProjectileCollisionSystemEntt* _projectileCollisionSystem = nullptr;
    cocos2d::EventListenerPhysicsContact* _sharedContactListener = nullptr;
    entt::entity _playerEntity = entt::null;

    cocos2d::Label* _monsterCountLabel = nullptr;
    float _monsterCountLabelTimer = 0.0f;
    float _monsterCountLabelInterval = 0.25f;

    FrameRateLimitMode _frameRateLimitMode = FrameRateLimitMode::Fps60;

    // UI Panels
    InventoryLayer* _inventoryLayer = nullptr;
    EquipmentPanel* _equipmentPanel = nullptr;
    CraftBar* _craftBar = nullptr;

    // 璁剧疆鐩稿叧鐨?UI 鍏冪礌
    cocos2d::Label* _saveNotification = nullptr;
    void showSaveNotification(const std::string& message);
};
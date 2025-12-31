#include "PlayerCraftingSystem.h"
#include "systems/item/StationDetector.h"
#include "systems/item/CraftingMatcher.h"
#include "systems/block_layer/block_layer.h"
#include "components/block/block_component.h"

USING_NS_CC;

// 静态成员初始化
float PlayerCraftingSystem::s_detectionRange = 200.0f;
bool PlayerCraftingSystem::s_craftingUIOpen = false;
std::vector<StationType> PlayerCraftingSystem::s_lastDetectedStations;

// ==================== PlayerCraftingSystem ====================

void PlayerCraftingSystem::update(entt::registry& registry, float dt) {
    // 查找玩家实体
    auto view = registry.view<ecs::PlayerTag, ecs::TransformComponent>();

    for (auto entity : view) {
        auto& transform = view.get<ecs::TransformComponent>(entity);
        Vec2 playerPos = transform.position;

        // 检测附近的工作台
        auto nearbyStations = detectNearbyStations(registry, playerPos);

        // 更新StationDetector
        updateStationDetector(nearbyStations);

        // 只处理第一个玩家实体
        break;
    }
}

std::vector<StationType> PlayerCraftingSystem::detectNearbyStations(entt::registry& registry, const cocos2d::Vec2& playerPos) {
    std::vector<StationType> detectedStations;

    // 始终添加Hand工作台(玩家手工合成)
    detectedStations.push_back(StationType::Hand);

    // 获取BlockWorld来检测方块类型的工作台
    if (!registry.ctx().contains<BlockWorld>()) {
        return detectedStations;
    }

    auto& blockWorld = registry.ctx().get<BlockWorld>();

    // 计算检测范围(以方块为单位)
    // 假设每个方块是16x16像素
    const float blockSize = 16.0f;
    int rangeInBlocks = static_cast<int>(s_detectionRange / blockSize);

    // 将玩家位置转换为方块坐标
    Vec2i playerBlockPos(
        static_cast<int>(playerPos.x / blockSize),
        static_cast<int>(playerPos.y / blockSize)
    );

    // 遍历玩家周围的方块
    for (int dy = -rangeInBlocks; dy <= rangeInBlocks; dy++) {
        for (int dx = -rangeInBlocks; dx <= rangeInBlocks; dx++) {
            Vec2i checkPos(playerBlockPos.x + dx, playerBlockPos.y + dy);

            // 检查距离是否在范围内(圆形检测)
            float distance = std::sqrt(dx * dx + dy * dy) * blockSize;
            if (distance > s_detectionRange) {
                continue;
            }

            // 获取该位置的方块
            BlockHandle blockHandle = blockWorld.getBlockAtBlockPos(LayerType::BLOCK, checkPos);
            if (!blockHandle.isIDVailed()) {
                continue;
            }

            // 根据方块ID判断工作台类型
            // 这里需要根据你的实际方块ID定义来映射
            // 假设工作台的方块ID为2101(根据ItemsTestScene.cpp)
            int blockId = static_cast<int>(blockHandle.id.value());

            switch (blockId) {
                case 2101:  // Workbench
                    if (std::find(detectedStations.begin(), detectedStations.end(),
                                  StationType::Workbench) == detectedStations.end()) {
                        detectedStations.push_back(StationType::Workbench);
                        CCLOG("PlayerCraftingSystem: Detected Workbench at (%d, %d)",
                              checkPos.x, checkPos.y);
                    }
                    break;

                case 2102:  // Furnace
                    if (std::find(detectedStations.begin(), detectedStations.end(),
                                  StationType::Furnace) == detectedStations.end()) {
                        detectedStations.push_back(StationType::Furnace);
                        CCLOG("PlayerCraftingSystem: Detected Furnace at (%d, %d)",
                              checkPos.x, checkPos.y);
                    }
                    break;

                case 2103:  // Anvil (如果有的话)
                    if (std::find(detectedStations.begin(), detectedStations.end(),
                                  StationType::Anvil) == detectedStations.end()) {
                        detectedStations.push_back(StationType::Anvil);
                        CCLOG("PlayerCraftingSystem: Detected Anvil at (%d, %d)",
                              checkPos.x, checkPos.y);
                    }
                    break;

                // 添加更多工作台类型...
                default:
                    break;
            }
        }
    }

    return detectedStations;
}

void PlayerCraftingSystem::updateStationDetector(const std::vector<StationType>& nearbyStations) {
    // 获取StationDetector单例
    auto* detector = StationDetector::getInstance();
    if (!detector) {
        return;
    }

    // 检查工作台列表是否发生变化
    bool stationsChanged = false;

    // 检查新增的工作台
    for (auto station : nearbyStations) {
        if (std::find(s_lastDetectedStations.begin(), s_lastDetectedStations.end(), station)
            == s_lastDetectedStations.end()) {
            // 新检测到的工作台
            detector->addStation(station);
            stationsChanged = true;
            CCLOG("PlayerCraftingSystem: Added station type %d", static_cast<int>(station));
        }
    }

    // 检查移除的工作台
    for (auto station : s_lastDetectedStations) {
        if (std::find(nearbyStations.begin(), nearbyStations.end(), station)
            == nearbyStations.end()) {
            // 玩家离开了工作台范围
            detector->removeStation(station);
            stationsChanged = true;
            CCLOG("PlayerCraftingSystem: Removed station type %d", static_cast<int>(station));
        }
    }

    // 更新上一帧的工作台列表
    s_lastDetectedStations = nearbyStations;

    // 如果工作台列表发生变化,触发事件通知UI更新
    if (stationsChanged) {
        // 获取可用配方数量用于调试
        auto matcher = CraftingMatcher::getInstance();
        auto recipes = matcher->getAvailableRecipes();
        CCLOG("PlayerCraftingSystem: Available recipes updated: %zu recipes", recipes.size());

        // 触发自定义事件
        EventCustom event("Event_CraftingStationsChanged");
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
    }
}

void PlayerCraftingSystem::setDetectionRange(float range) {
    s_detectionRange = range;
}

float PlayerCraftingSystem::getDetectionRange() {
    return s_detectionRange;
}

void PlayerCraftingSystem::openCraftingUI() {
    if (!s_craftingUIOpen) {
        s_craftingUIOpen = true;
        CCLOG("PlayerCraftingSystem: Crafting UI opened");

        // 触发打开合成界面事件
        EventCustom event("Event_OpenCraftingUI");
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
    }
}

void PlayerCraftingSystem::closeCraftingUI() {
    if (s_craftingUIOpen) {
        s_craftingUIOpen = false;
        CCLOG("PlayerCraftingSystem: Crafting UI closed");

        // 触发关闭合成界面事件
        EventCustom event("Event_CloseCraftingUI");
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
    }
}

void PlayerCraftingSystem::toggleCraftingUI() {
    if (s_craftingUIOpen) {
        closeCraftingUI();
    } else {
        openCraftingUI();
    }
}

bool PlayerCraftingSystem::isCraftingUIOpen() {
    return s_craftingUIOpen;
}

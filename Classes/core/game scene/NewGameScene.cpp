#include "NewGameScene.h"
#include "core/scenes/MainMenuScene.h"
#include "systems/save/SaveData.h"
#include "utils/audio/AudioManager.h"       
#include "ui/CocosGUI.h"
#include "systems/ECS/ECSSystem.h"
#include <core/world.h>


USING_NS_CC;

/*
                   _ooOoo_
                  o8888888o
                  88" . "88
                  (| -_- |)
                  O\  =  /O
               ____/`---'\____
             .'  \\|     |//  `.
            /  \\|||  :  |||//  \
           /  _||||| -:- |||||-  \
           |   | \\\  -  /// |   |
           | \_|  ''\---/''  |   |
           \  .-\__  `-`  ___/-. /
         ___`. .'  /--.--\  `. . __
      ."" '<  `.___\_<|>_/___.'  >'"".
     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
     \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
                   `=---='
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
           佛祖保佑       永无BUG
*/

// 自动保存相关成员
bool m_autoSaveEnabled = false;              // 当前场景是否启用自动保存
int  m_autoSaveIntervalMinutes = 5;          // 间隔（分钟），默认 5

// 事件监听句柄（用于监听来自设置界面的事件）
cocos2d::EventListenerCustom* m_autoSaveEnabledListener = nullptr;
cocos2d::EventListenerCustom* m_autoSaveIntervalListener = nullptr;

Scene* NewGameScene::createScene()
{
    return NewGameScene::create();
}

bool NewGameScene::init()
{
    if (!Scene::init())
        return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ----------------------------
    // 背景
    // ----------------------------
    auto bg = Sprite::create("bg/World refference/Forest/Forest_background_1.png");
    bg->setPosition(visibleSize / 2);
    float scaleX = visibleSize.width / bg->getContentSize().width;
    float scaleY = visibleSize.height / bg->getContentSize().height;
    bg->setScale(MAX(scaleX, scaleY));
    this->addChild(bg, -1);

    // ----------------------------
    // 初始化存档系统
    // ----------------------------
    initSaveSystem();

    // ----------------------------
    // 创建玩家角色
    // ----------------------------
    createPlayer();

    // ----------------------------
        // Setting 按钮
        // ----------------------------
    settingLabel = Label::createWithTTF("Setting", "fonts/Marker Felt.ttf", 36);
    settingLabel->setPosition(Vec2(100, visibleSize.height - 50));
    this->addChild(settingLabel);
    settingLabel->setName("SettingLabel");

    // Setting 按钮触摸监听器
    auto settingListener = EventListenerTouchOneByOne::create();
    settingListener->setSwallowTouches(true);
    settingListener->onTouchBegan = [=](Touch* t, Event* e) mutable {
        if (!isSettingEnabled) {
            CCLOG("Setting button is disabled!");
            return false;
        }

        if (settingLabel->getBoundingBox().containsPoint(t->getLocation()))
        {
            // 禁用Setting按钮
            updateSettingButtonState(false);

            // 黑色覆盖层 - 覆盖整个屏幕，z-order较低
            auto darkLayer = LayerColor::create(Color4B(0, 0, 0, 150));
            darkLayer->setContentSize(visibleSize);
            darkLayer->setPosition(Vec2::ZERO);
            darkLayer->setName("DarkLayer");
            this->addChild(darkLayer, 5);

            // 覆盖层触摸监听 - 防止穿透到下层
            auto menuTouchListener = EventListenerTouchOneByOne::create();
            menuTouchListener->setSwallowTouches(true);
            menuTouchListener->onTouchBegan = [=](Touch* t, Event* e) {
                return true; // 吞噬所有触摸事件
                };
            Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(menuTouchListener, darkLayer);

            // 菜单容器节点 - z-order较高
            auto menuNode = Node::create();
            menuNode->setName("MenuNode");
            this->addChild(menuNode, 10);

            auto panel = LayerColor::create(Color4B(50, 50, 50, 200), 500, 400);
            panel->setPosition(visibleSize.width / 2 - 250, visibleSize.height / 2 - 200);
            menuNode->addChild(panel);

            // Resume 按钮
            auto resumeLabel = Label::createWithTTF("Resume", "fonts/Marker Felt.ttf", 36);
            resumeLabel->setPosition(panel->getContentSize().width / 2, 280);
            resumeLabel->setName("ResumeLabel");
            panel->addChild(resumeLabel);

            // Save 按钮
            auto saveLabel = Label::createWithTTF("Save Game", "fonts/Marker Felt.ttf", 36);
            saveLabel->setPosition(panel->getContentSize().width / 2, 200);
            saveLabel->setColor(Color3B::GREEN);
            panel->addChild(saveLabel);

            // Load 按钮
            auto loadLabel = Label::createWithTTF("Load Game", "fonts/Marker Felt.ttf", 36);
            loadLabel->setPosition(panel->getContentSize().width / 2, 120);
            panel->addChild(loadLabel);

            // Quit 按钮
            auto quitBtn = Label::createWithTTF("Quit to Menu", "fonts/Marker Felt.ttf", 36);
            quitBtn->setPosition(panel->getContentSize().width / 2, 40);
            panel->addChild(quitBtn);

            // Resume 按钮监听器
            auto resumeListener = EventListenerTouchOneByOne::create();
            resumeListener->setSwallowTouches(true);
            resumeListener->onTouchBegan = [=](Touch* t, Event* e) mutable {
                Vec2 locationInNode = resumeLabel->convertTouchToNodeSpace(t);
                Size s = resumeLabel->getContentSize();
                Rect rect = Rect(0, 0, s.width, s.height);

                if (rect.containsPoint(locationInNode))
                {
                    CCLOG("Resume clicked!");

                    // 移除事件监听器
                    Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(darkLayer);

                    // 移除覆盖层和菜单
                    darkLayer->removeFromParent();
                    menuNode->removeFromParent();

                    // 恢复Setting按钮
                    updateSettingButtonState(true);
                    // 切换到游戏场景
                    Director::getInstance()->replaceScene(
                        TransitionFade::create(1.0f, World::createScene())
                    );
                    return true;
                }
                return false;
                };
            Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(resumeListener, resumeLabel);

            // Save 按钮监听器
            auto saveListener = EventListenerTouchOneByOne::create();
            saveListener->setSwallowTouches(true);
            saveListener->onTouchBegan = [=](Touch* t, Event* e) mutable {
                Vec2 locationInNode = saveLabel->convertTouchToNodeSpace(t);
                Size s = saveLabel->getContentSize();
                Rect rect = Rect(0, 0, s.width, s.height);

                if (rect.containsPoint(locationInNode))
                {
                    CCLOG("Save button clicked!");

                    // 保存当前游戏
                    saveCurrentGame();

                    // 移除覆盖层和菜单
                    darkLayer->removeFromParent();
                    menuNode->removeFromParent();

                    // 恢复Setting按钮
                    updateSettingButtonState(true);
                    // 切换到游戏场景
                    Director::getInstance()->replaceScene(
                        TransitionFade::create(1.0f, World::createScene())
                    );
                    return true;
                }
                return false;
                };
            Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(saveListener, saveLabel);

            // Load 按钮监听器
            auto loadListener = EventListenerTouchOneByOne::create();
            loadListener->setSwallowTouches(true);
            loadListener->onTouchBegan = [=](Touch* t, Event* e) mutable {
                Vec2 locationInNode = loadLabel->convertTouchToNodeSpace(t);
                Size s = loadLabel->getContentSize();
                Rect rect = Rect(0, 0, s.width, s.height);

                if (rect.containsPoint(locationInNode))
                {
                    CCLOG("Load button clicked!");

                    // 移除覆盖层和菜单
                    darkLayer->removeFromParent();
                    menuNode->removeFromParent();

                    // 打开存档/读档菜单
                    createSaveLoadMenu();

                    return true;
                }
                return false;
                };
            Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(loadListener, loadLabel);

            // Quit 按钮监听器
            auto quitListener = EventListenerTouchOneByOne::create();
            quitListener->setSwallowTouches(true);
            quitListener->onTouchBegan = [=](Touch* t, Event* e) mutable {
                Vec2 locationInNode = quitBtn->convertTouchToNodeSpace(t);
                Size s = quitBtn->getContentSize();
                Rect rect = Rect(0, 0, s.width, s.height);

                if (rect.containsPoint(locationInNode))
                {
                    CCLOG("Quit clicked!");
                    // 这里可以跳转到主菜单
                    Director::getInstance()->replaceScene(
                        TransitionFade::create(0.5f, MainMenuScene::createScene()));
                    return true;
                }
                return false;
                };
            Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(quitListener, quitBtn);

            return true;
        }
        return false;
        };

    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(settingListener, settingLabel);

    // -------------------------
    // 自动保存：读取 AudioManager 并应用
    // -------------------------
    // 从 AudioManager 读取当前设置
    m_autoSaveEnabled = AudioManager::isAutoSaveEnabled();
    m_autoSaveIntervalMinutes = AudioManager::getAutoSaveInterval();

    // 立即根据设置启动/停止定时器
    applyAutoSaveSettings();

    // 监听来自设置界面的变更（MainMenuScene 在更改设置后应调用下面的 dispatchCustomEvent）
    // 事件名："auto_save_changed" 和 "auto_save_interval_changed"
    m_autoSaveEnabledListener = Director::getInstance()->getEventDispatcher()
        ->addCustomEventListener("auto_save_changed", [this](EventCustom* ev) {
        // 读取新的值并应用
        m_autoSaveEnabled = AudioManager::isAutoSaveEnabled();
        applyAutoSaveSettings();
        CCLOG("NewGameScene received auto_save_changed => %d", m_autoSaveEnabled);
            });

    m_autoSaveIntervalListener = Director::getInstance()->getEventDispatcher()
        ->addCustomEventListener("auto_save_interval_changed", [this](EventCustom* ev) {
        m_autoSaveIntervalMinutes = AudioManager::getAutoSaveInterval();
        applyAutoSaveSettings(); // 重新 schedule
        CCLOG("NewGameScene received auto_save_interval_changed => %d minutes", m_autoSaveIntervalMinutes);
            });

    return true;
}

void NewGameScene::enableAutoSave(bool enable)
{
    AudioManager::setAutoSaveEnabled(enable);
    // 立即生效
    m_autoSaveEnabled = enable;
    applyAutoSaveSettings();

    // 通知其他可能的监听者（例如其他场景）
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("auto_save_changed");
}

void NewGameScene::setAutoSaveIntervalMinutes(int minutes)
{
    AudioManager::setAutoSaveInterval(minutes);
    m_autoSaveIntervalMinutes = minutes;
    applyAutoSaveSettings();
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("auto_save_interval_changed");
}

void NewGameScene::applyAutoSaveSettings()
{
    // 先取消任何已有的 schedule
    this->unschedule(CC_SCHEDULE_SELECTOR(NewGameScene::autoSaveTick));

    if (m_autoSaveEnabled) {
        // 将分钟转换为秒
        float seconds = std::max(1, m_autoSaveIntervalMinutes) * 60.0f;
        this->schedule(CC_SCHEDULE_SELECTOR(NewGameScene::autoSaveTick), seconds);
        CCLOG("AutoSave scheduled every %.1f seconds", seconds);
    }
    else {
        CCLOG("AutoSave disabled in NewGameScene");
    }
}

void NewGameScene::autoSaveTick(float dt)
{
    CCLOG("[AutoSave] tick (dt=%.2f) - saving", dt);

    // 如果场景当前正处于弹窗（isSaveLoadMenuActive）可能不希望自动保存，判断一下
    if (isSaveLoadMenuActive) {
        CCLOG("[AutoSave] skipped because Save/Load menu active");
        return;
    }

    // 执行保存
    saveCurrentGame();
    // 可以显示更轻的提示
    showSaveNotification("Auto Saved");
}

void NewGameScene::updateSettingButtonState(bool enabled)
{
    isSettingEnabled = enabled;

    if (settingLabel) {
        if (enabled) {
            settingLabel->setColor(Color3B::WHITE);
            settingLabel->setOpacity(255);
        }
        else {
            settingLabel->setColor(Color3B::GRAY);
            settingLabel->setOpacity(150);
        }
    }
}

// ==================== ECS 相关函数 ====================
void NewGameScene::initECS() {
    ecsRegistry.clear();
    playerEntity = entt::null;
}

void NewGameScene::createOrUpdatePlayerEntity() {
    if (playerEntity == entt::null || !ecsRegistry.valid(playerEntity)) {
        // 创建玩家实体
        playerEntity = ecsRegistry.create();

        // 添加必要的组件
        ecsRegistry.emplace<ECS::Transform>(playerEntity);
        ecsRegistry.emplace<ECS::PlayerComponent>(playerEntity);

        // 添加标签以便查找
        ECS::TagComponent tag;
        tag.tag = "player";
        ecsRegistry.emplace<ECS::TagComponent>(playerEntity, tag);

        CCLOG("Player entity created");
    }
}

void NewGameScene::syncSpriteToECS() {
    if (player) {
        createOrUpdatePlayerEntity();

        // 更新 Transform 组件
        auto& transform = ecsRegistry.get<ECS::Transform>(playerEntity);
        transform.x = player->getPositionX();
        transform.y = player->getPositionY();

        // 更新存档数据
        currentSaveData.playerPos.position.x = transform.x;
        currentSaveData.playerPos.position.y = transform.y;

        CCLOG("Sprite position synced to ECS: %.2f, %.2f", transform.x, transform.y);
    }
}

void NewGameScene::syncECSToSprite() {
    if (player && playerEntity != entt::null && ecsRegistry.valid(playerEntity)) {
        if (ecsRegistry.all_of<ECS::Transform>(playerEntity)) {
            const auto& transform = ecsRegistry.get<ECS::Transform>(playerEntity);
            player->setPosition(transform.x, transform.y);

            currentSaveData.playerPos.position.x = transform.x;
            currentSaveData.playerPos.position.y = transform.y;

            CCLOG("ECS position synced to sprite: %.2f, %.2f", transform.x, transform.y);
        }
    }
}

// ==================== 存档系统相关函数 ====================

void NewGameScene::initSaveSystem() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    currentSaveSlot = UserDefault::getInstance()->getIntegerForKey("load_slot", 0);
    bool isNewGame = UserDefault::getInstance()->getBoolForKey("is_new_game", false);

    CCLOG("Current save slot: %d, isNewGame: %d", currentSaveSlot, isNewGame);

    // 初始化 ECS
    initECS();

    if (currentSaveSlot > 0) {
        if (isNewGame) {
            // 新游戏：设置默认数据
            currentSaveData.slotNumber = currentSaveSlot;
            currentSaveData.saveName = "My Save " + std::to_string(currentSaveSlot);
            currentSaveData.saveTime = time(nullptr);
            currentSaveData.playerName = "Player";
            currentSaveData.playerLevel = 1;
            currentSaveData.playerExp = 0;
            currentSaveData.playerPos.position.x = visibleSize.width / 2;
            currentSaveData.playerPos.position.y = 200;
            currentSaveData.playerHealth = 100;
            currentSaveData.playerMaxHealth = 100;
            currentSaveData.playerCoins = 0;
            currentSaveData.playerMana = 100;
            currentSaveData.playerMaxMana = 100;

            // 创建玩家实体并应用数据
            createOrUpdatePlayerEntity();
            EnttSerializer::applyToEntity(ecsRegistry, playerEntity, currentSaveData);

            // 保存初始存档
            SimpleSaveManager::getInstance()->saveGameWithEntt(
                currentSaveSlot, ecsRegistry, playerEntity);

            // 清除新游戏标记
            UserDefault::getInstance()->setBoolForKey("is_new_game", false);
            UserDefault::getInstance()->flush();

            CCLOG("New game created with EnTT in slot %d", currentSaveSlot);
        }
        else {
            // 加载已有存档
            loadGameData();
        }
    }
    else {
        // 没有指定存档槽，使用默认
        currentSaveSlot = 1;
        currentSaveData.slotNumber = currentSaveSlot;
        currentSaveData.saveName = "Default Save";
        currentSaveData.saveTime = time(nullptr);
        currentSaveData.playerName = "Player";
        currentSaveData.playerLevel = 1;
        currentSaveData.playerExp = 0;
        currentSaveData.playerPos.position.x = visibleSize.width / 2;
        currentSaveData.playerPos.position.y = 200;
        currentSaveData.playerHealth = 100;
        currentSaveData.playerMaxHealth = 100;
        currentSaveData.playerCoins = 0;
        currentSaveData.playerMana = 100;
        currentSaveData.playerMaxMana = 100;
        
        // 创建玩家实体并应用默认数据
        createOrUpdatePlayerEntity();
        EnttSerializer::applyToEntity(ecsRegistry, playerEntity, currentSaveData);

        CCLOG("Using default EnTT save data");
    }
}

void NewGameScene::loadGameData() {
    // 确保有玩家实体
    createOrUpdatePlayerEntity();

    if (SimpleSaveManager::getInstance()->loadGameWithEntt(
        currentSaveSlot, ecsRegistry, playerEntity)) {

        CCLOG("Game loaded with EnTT from slot %d", currentSaveSlot);

        // 从 ECS 更新当前存档数据
        if (ecsRegistry.all_of<ECS::PlayerComponent>(playerEntity)) {
            const auto& playerComp = ecsRegistry.get<ECS::PlayerComponent>(playerEntity);
            currentSaveData.playerName = playerComp.name;
            currentSaveData.playerLevel = playerComp.level;
            currentSaveData.playerExp = playerComp.exp;
            currentSaveData.playerHealth = playerComp.health;
            currentSaveData.playerMaxHealth = playerComp.maxHealth;
            currentSaveData.playerCoins = playerComp.coins;
            currentSaveData.playerMana = playerComp.mana;
            currentSaveData.playerMaxMana = playerComp.maxMana;
        }

        // 同步精灵位置
        syncECSToSprite();

        CCLOG("Player position loaded: %.2f, %.2f",
            currentSaveData.playerPos.position.x, currentSaveData.playerPos.position.y);
        CCLOG("Player level: %d, HP: %d/%d",
            currentSaveData.playerLevel,
            currentSaveData.playerHealth, currentSaveData.playerMaxHealth);
    }
    else {
        CCLOG("Failed to load game from slot %d", currentSaveSlot);

        // 加载失败时使用默认数据
        currentSaveData.playerLevel = 1;
        currentSaveData.playerExp = 0;
        currentSaveData.playerHealth = 100;
        currentSaveData.playerMaxHealth = 100;
        currentSaveData.playerCoins = 0;
        currentSaveData.playerMana = 100;
        currentSaveData.playerMaxMana = 100;

        auto visibleSize = Director::getInstance()->getVisibleSize();
        currentSaveData.playerPos.position.x = visibleSize.width / 2;
        currentSaveData.playerPos.position.y = 200;

        // 应用默认数据到 ECS
        EnttSerializer::applyToEntity(ecsRegistry, playerEntity, currentSaveData);
        syncECSToSprite();
    }
}

SimpleSaveData NewGameScene::getCurrentGameState() {
    SimpleSaveData state = currentSaveData;
    state.saveTime = time(nullptr);

    // 同步精灵位置到 ECS
    syncSpriteToECS();

    // 从 ECS 获取最新数据
    if (playerEntity != entt::null && ecsRegistry.valid(playerEntity)) {
        if (ecsRegistry.all_of<ECS::Transform>(playerEntity)) {
            const auto& transform = ecsRegistry.get<ECS::Transform>(playerEntity);
            state.playerPos.position.x = transform.x;
            state.playerPos.position.y = transform.y;
        }

        if (ecsRegistry.all_of<ECS::PlayerComponent>(playerEntity)) {
            const auto& playerComp = ecsRegistry.get<ECS::PlayerComponent>(playerEntity);
            state.playerName = playerComp.name;
            state.playerLevel = playerComp.level;
            state.playerExp = playerComp.exp;
            state.playerHealth = playerComp.health;
            state.playerMaxHealth = playerComp.maxHealth;
            state.playerCoins = playerComp.coins;
            state.playerMana = playerComp.mana;
            state.playerMaxMana = playerComp.maxMana;
        }
    }

    return state;
}

void NewGameScene::saveCurrentGame() {
    // 确保数据同步
    syncSpriteToECS();

    SimpleSaveData currentState = getCurrentGameState();
    currentState.slotNumber = currentSaveSlot;

    CCLOG("Saving game to slot %d", currentSaveSlot);
    CCLOG("Player position: %.2f, %.2f", currentState.playerPos.position.x, currentState.playerPos.position.y);
    CCLOG("Player stats: Lv.%d HP:%d/%d Coins:%d",
        currentState.playerLevel, currentState.playerHealth,
        currentState.playerMaxHealth, currentState.playerCoins);

    if (SimpleSaveManager::getInstance()->saveGameWithEntt(
        currentSaveSlot, ecsRegistry, playerEntity)) {

        // 更新当前存档数据
        currentSaveData = currentState;

        // 显示保存通知
        showSaveNotification("Game Saved!");

        CCLOG("Game saved successfully to slot %d", currentSaveSlot);
    }
    else {
        showSaveNotification("Save Failed!");
        CCLOG("Failed to save game to slot %d", currentSaveSlot);
    }
}

void NewGameScene::showSaveNotification(const std::string& message)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    auto notification = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 24);
    notification->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    notification->setColor(Color3B::GREEN);
    this->addChild(notification, 100);
    notification->setName("SaveNotification");

    // 淡出效果
    auto fadeOut = FadeOut::create(2.0f);
    auto remove = CallFunc::create([notification]() {
        notification->removeFromParent();
        });
    notification->runAction(Sequence::create(
        DelayTime::create(1.0f),
        fadeOut,
        remove,
        nullptr
    ));
}

// ==================== 存档/读档菜单相关函数 ====================

void NewGameScene::createSaveLoadMenu()
{
    auto saveLoadScene = Scene::create();

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 背景覆盖
    auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 200));
    bgLayer->setContentSize(visibleSize);
    saveLoadScene->addChild(bgLayer, 0);

    // 标题
    auto titleLabel = Label::createWithTTF("Save/Load Game", "fonts/Marker Felt.ttf", 36);
    titleLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    titleLabel->setColor(Color3B::YELLOW);
    saveLoadScene->addChild(titleLabel, 1);

    // 获取所有存档
    auto saveManager = SimpleSaveManager::getInstance();
    auto saves = saveManager->getAllSaves();

    // 创建6个存档槽位
    for (int i = 0; i < 6; i++) {
        int slotNumber = i + 1;

        // 存档槽背景
        auto slotBg = Sprite::create("bg/loadoption.png");
        slotBg->setColor(saves[i].isEmpty() ? Color3B(80, 80, 80) : Color3B(60, 60, 60));
        slotBg->setContentSize(Size(350, 90));
        slotBg->setOpacity(200);

        // 计算位置 (2x3网格)
        int row = i / 2;
        int col = i % 2;

        float posX = visibleSize.width * (0.25f + col * 0.5f);
        float posY = visibleSize.height * 0.7f - row * 180;

        slotBg->setPosition(Vec2(posX, posY));
        slotBg->setName("SlotBg_" + std::to_string(slotNumber));
        saveLoadScene->addChild(slotBg, 1);

        // 如果是当前存档槽，高亮显示
        if (slotNumber == currentSaveSlot) {
            auto highlight = Sprite::create();
            highlight->setColor(Color3B::YELLOW);
            highlight->setContentSize(Size(360, 100));
            highlight->setOpacity(100);
            highlight->setPosition(slotBg->getPosition());
            saveLoadScene->addChild(highlight, 0);
        }

        // 存档信息
        std::string slotText = saves[i].getDisplayText();
        if (slotNumber == currentSaveSlot) {
            slotText += "\n[Current]";
        }

        auto slotLabel = Label::createWithTTF(slotText, "fonts/Marker Felt.ttf", 20);
        slotLabel->setPosition(slotBg->getPosition());
        slotLabel->setDimensions(330, 80);
        slotLabel->setHorizontalAlignment(TextHAlignment::CENTER);
        slotLabel->setVerticalAlignment(TextVAlignment::CENTER);
        slotLabel->setColor(saves[i].isEmpty() ? Color3B(180, 180, 180) : Color3B::WHITE);
        slotLabel->setName("SlotLabel_" + std::to_string(slotNumber));
        saveLoadScene->addChild(slotLabel, 2);

        // 存档槽位点击事件
        auto slotListener = EventListenerTouchOneByOne::create();
        slotListener->setSwallowTouches(true);
        slotListener->onTouchBegan = [=](Touch* t, Event* e) {
            // 检查是否有激活的对话框
            if (isSaveLoadMenuActive) {
                CCLOG("Cannot select slot while dialog is active!");
                return false;
            }

            Rect bbox = slotBg->getBoundingBox();
            if (bbox.containsPoint(t->getLocation())) {
                if (saves[i].isEmpty()) {
                    // 空存档槽 - 保存当前游戏
                    showSimpleSaveDialog(saveLoadScene, slotNumber);
                }
                else {
                    // 已有存档 - 显示操作选择
                    showSaveLoadDialog(saveLoadScene, slotNumber, saves[i]);
                }
                return true;
            }
            return false;
            };
        saveLoadScene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(slotListener, slotBg);
    }

    // 返回按钮
    auto backLabel = Label::createWithTTF("Back to Game", "fonts/Marker Felt.ttf", 36);
    backLabel->setPosition(Vec2(visibleSize.width / 2, 80));
    backLabel->setColor(Color3B::GREEN);
    saveLoadScene->addChild(backLabel, 10);

    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [=](Touch* t, Event* e) {
        if (backLabel->getBoundingBox().containsPoint(t->getLocation())) {
            // 恢复Setting按钮状态
            isSaveLoadMenuActive = false;
            updateSettingButtonState(true);

            // 切换到游戏场景
            Director::getInstance()->replaceScene(
                TransitionFade::create(1.0f, World::createScene())
            );
            return true;
        }
        return false;
        };
    saveLoadScene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, backLabel);

    // 推送到场景栈
    Director::getInstance()->pushScene(saveLoadScene);
}
void NewGameScene::showSimpleSaveDialog(Scene* parentScene, int slotNumber)
{
    // 设置对话框激活状态
    isSaveLoadMenuActive = true;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 对话框背景
    auto dialogBg = LayerColor::create(Color4B(40, 40, 40, 230), 350, 150);
    dialogBg->setPosition(visibleSize.width / 2 - 175, visibleSize.height / 2 - 75);
    dialogBg->setName("SaveDialog");
    parentScene->addChild(dialogBg, 100);

    // 提示信息
    auto infoLabel = Label::createWithTTF("Save to Slot " + std::to_string(slotNumber) + "?", "fonts/Marker Felt.ttf", 24);
    infoLabel->setPosition(175, 100);
    infoLabel->setColor(Color3B::WHITE);
    dialogBg->addChild(infoLabel);

    // 使用MenuItemLabel创建按钮
    auto confirmItem = MenuItemLabel::create(
        Label::createWithTTF("Save", "fonts/Marker Felt.ttf", 28),
        [=](Ref* sender) {
            CCLOG("Save button clicked for slot %d", slotNumber);

            // 保存当前游戏到指定槽位
            currentSaveSlot = slotNumber;
            saveCurrentGame();

            // 关闭对话框
            dialogBg->removeFromParent();

            // 恢复存档选择状态
            isSaveLoadMenuActive = false;

        });
    confirmItem->setColor(Color3B::GREEN);
    confirmItem->setPosition(120, 50);

    auto cancelItem = MenuItemLabel::create(
        Label::createWithTTF("Cancel", "fonts/Marker Felt.ttf", 28),
        [=](Ref* sender) {
            CCLOG("Cancel button clicked");

            // 关闭对话框
            dialogBg->removeFromParent();

            // 恢复存档选择状态
            isSaveLoadMenuActive = false;
        });
    cancelItem->setColor(Color3B::YELLOW);
    cancelItem->setPosition(230, 50);

    // 创建菜单
    auto menu = Menu::create(confirmItem, cancelItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    dialogBg->addChild(menu);
}
void NewGameScene::showSaveLoadDialog(Scene* parentScene, int slotNumber, const SimpleSaveData& saveData)
{
    // 设置对话框激活状态
    isSaveLoadMenuActive = true;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 对话框背景
    auto dialogBg = LayerColor::create(Color4B(40, 40, 40, 230), 400, 220);
    dialogBg->setPosition(visibleSize.width / 2 - 200, visibleSize.height / 2 - 110);
    dialogBg->setName("SaveLoadDialog");
    parentScene->addChild(dialogBg, 100);

    // 存档信息
    auto infoLabel = Label::createWithTTF(saveData.getDisplayText(), "fonts/Marker Felt.ttf", 20);
    infoLabel->setPosition(200, 160);
    infoLabel->setDimensions(380, 80);
    infoLabel->setHorizontalAlignment(TextHAlignment::CENTER);
    infoLabel->setColor(Color3B::WHITE);
    dialogBg->addChild(infoLabel);

    // 使用MenuItemLabel创建按钮
    auto saveItem = MenuItemLabel::create(
        Label::createWithTTF("Save (Overwrite)", "fonts/Marker Felt.ttf", 22),
        [=](Ref* sender) {
            CCLOG("Save button clicked!");

            // 保存当前游戏（覆盖）
            currentSaveSlot = slotNumber;
            saveCurrentGame();

            // 关闭对话框
            dialogBg->removeFromParent();

            // 恢复存档选择状态
            isSaveLoadMenuActive = false;

            // 恢复Setting按钮
            updateSettingButtonState(true);

            // 切换到游戏场景
            Director::getInstance()->replaceScene(
                TransitionFade::create(1.0f, World::createScene())
            );
        });
    saveItem->setColor(Color3B::GREEN);
    saveItem->setPosition(130, 90);

    auto loadItem = MenuItemLabel::create(
        Label::createWithTTF("Load", "fonts/Marker Felt.ttf", 22),
        [=](Ref* sender) {
            CCLOG("Load button clicked!");

            // 加载存档
            currentSaveSlot = slotNumber;
            loadGameData();

            // 关闭对话框
            dialogBg->removeFromParent();

            // 恢复存档选择状态
            isSaveLoadMenuActive = false;

            // 恢复Setting按钮
            updateSettingButtonState(true);


            // 切换到游戏场景
    Director::getInstance()->replaceScene(
        TransitionFade::create(1.0f, World::createScene())
    );

            // 显示加载成功通知
            showSaveNotification("Game Loaded!");
        });
    loadItem->setColor(Color3B::BLUE);
    loadItem->setPosition(270, 90);

    auto deleteItem = MenuItemLabel::create(
        Label::createWithTTF("Delete", "fonts/Marker Felt.ttf", 22),
        [=](Ref* sender) {
            CCLOG("Delete button clicked!");

            // 删除存档
            SimpleSaveManager::getInstance()->deleteGame(slotNumber);

            // 关闭对话框
            dialogBg->removeFromParent();

            // 恢复存档选择状态
            isSaveLoadMenuActive = false;

            // 刷新界面
            createSaveLoadMenu();
        });
    deleteItem->setColor(Color3B::RED);
    deleteItem->setPosition(130, 40);

    auto cancelItem = MenuItemLabel::create(
        Label::createWithTTF("Cancel", "fonts/Marker Felt.ttf", 22),
        [=](Ref* sender) {
            CCLOG("Cancel button clicked!");

            // 关闭对话框
            dialogBg->removeFromParent();

            // 恢复存档选择状态
            isSaveLoadMenuActive = false;
        });
    cancelItem->setColor(Color3B::YELLOW);
    cancelItem->setPosition(270, 40);

    // 创建菜单
    auto menu = Menu::create(saveItem, loadItem, deleteItem, cancelItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    dialogBg->addChild(menu);
}

// ==================== 游戏功能函数 ====================

void NewGameScene::createPlayer() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建玩家精灵
    player = Sprite::create(); // 你的玩家图片路径
    if (!player) {
        player = Sprite::create();
        player->setColor(Color3B::RED);
        player->setContentSize(Size(50, 50));
        CCLOG("Using default player sprite");
    }

    // 设置位置（从 ECS 或存档数据）
    if (playerEntity != entt::null && ecsRegistry.valid(playerEntity) &&
        ecsRegistry.all_of<ECS::Transform>(playerEntity)) {
        syncECSToSprite();
    }
    else {
        player->setPosition(currentSaveData.playerPos.position.x, currentSaveData.playerPos.position.y);
    }

    player->setName("Player");
    this->addChild(player, 10);

    CCLOG("Player created at position: %.2f, %.2f",
        player->getPositionX(), player->getPositionY());

    // 修改触摸监听器，同时更新 ECS
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = [this](Touch* t, Event* e) {
        if (player) {
            Vec2 touchPos = t->getLocation();
            player->setPosition(touchPos);

            // 同步到 ECS
            syncSpriteToECS();

            CCLOG("Player moved to: %.2f, %.2f", touchPos.x, touchPos.y);
        }
        return true;
        };
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);
}
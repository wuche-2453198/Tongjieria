#include "MainMenuScene.h"
#include "audio/include/AudioEngine.h"
#include "ui/CocosGUI.h"
#include "utils/audio/AudioManager.h"
#include "core/world.h"   
#include "systems/save/SaveData.h"

USING_NS_CC;

Scene* MainMenuScene::createScene()
{
    return MainMenuScene::create();
}

// 错误处理辅助函数
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
}

bool MainMenuScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    // 在init开始时加载音频设置
    AudioManager::loadSettings();

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建背景
    auto background = Sprite::create("bg/World refference/Forest/Forest_background_1.png");
    this->addChild(background, 0);

    if (background == nullptr)
    {
        problemLoading("bg/World refference/Forest/Forest_background_1.png");
        return false;
    }

    else
    {
        // 将图片放在屏幕中心
        background->setPosition(Vec2(visibleSize.width / 2 + origin.x,
            visibleSize.height / 2 + origin.y));

        // 根据屏幕大小调整图片缩放比例，使其填满屏幕
        float scaleX = visibleSize.width / background->getContentSize().width;
        float scaleY = visibleSize.height / background->getContentSize().height;
        float scale = MAX(scaleX, scaleY);
        background->setScale(scale);
    }
    // 创建logo精灵
    auto logo = Sprite::create("bg/Tongjierialogo.png");
    // 设置位置到屏幕上方
    logo->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 4 * 3));
    // 添加到当前层
    this->addChild(logo, 1);

    // "New Game" 按钮
    auto startGameItem = MenuItemLabel::create(Label::createWithTTF("New Game", "fonts/Marker Felt.ttf", 36),
        [](Ref* pSender) {
            // 清理之前的存档标记
            UserDefault::getInstance()->setIntegerForKey("load_slot", 0);
            UserDefault::getInstance()->setBoolForKey("is_new_game", false);
            UserDefault::getInstance()->flush();

            // 直接开始新游戏（使用默认槽位1）
            // todo 修改成游戏世界类
            auto gameScene = World::createScene();
            Director::getInstance()->replaceScene(
                TransitionFade::create(1.0f, gameScene)
            );
        });
    if (startGameItem)
    {
        startGameItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 4 * 3 - logo->getContentSize().height - 10));
    }

    // "Load Game" 按钮
    auto loadItem = MenuItemLabel::create(Label::createWithTTF("Load Game", "fonts/Marker Felt.ttf", 36),
        CC_CALLBACK_1(MainMenuScene::menuLoadCallback, this));
    if (loadItem)
    {
        loadItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 4 * 3 - logo->getContentSize().height - 20
            - startGameItem->getContentSize().height));
    }

    // "Setting" 按钮
    auto setGameItem = MenuItemLabel::create(Label::createWithTTF("Setting", "fonts/Marker Felt.ttf", 36),
        CC_CALLBACK_1(MainMenuScene::menuSetGameCallback, this));
    if (setGameItem)
    {
        setGameItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 4 * 3 - logo->getContentSize().height - 30
            - loadItem->getContentSize().height - startGameItem->getContentSize().height));
    }

    // "Exit" 按钮
    auto closeItem = MenuItemLabel::create(Label::createWithTTF("Exit", "fonts/Marker Felt.ttf", 36),
        CC_CALLBACK_1(MainMenuScene::menuCloseCallback, this));
    if (closeItem)
    {
        closeItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 4 * 3 - logo->getContentSize().height - 40
            - loadItem->getContentSize().height - startGameItem->getContentSize().height
            - setGameItem->getContentSize().height));
    }

    // 创建菜单
    auto menu = Menu::create(startGameItem, loadItem, setGameItem, closeItem, nullptr);
    menu->setPosition(Vec2::ZERO); // 菜单的定位会通过其子项的setPosition来控制
    this->addChild(menu, 1); // Z-order 1，在背景之上

    return true;
}

void MainMenuScene::menuNewGameCallback(Ref* pSender)
{
    // 切换到游戏场景
    Director::getInstance()->replaceScene(
        TransitionFade::create(1.0f, World::createScene())
    );
}

void MainMenuScene::menuSetGameCallback(Ref* pSender)
{
    // 先加载当前音频设置
    AudioManager::loadSettings();

    auto setScene = Scene::create();
    Director::getInstance()->replaceScene(TransitionFade::create(1.0, setScene));

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 背景
    auto bg = Sprite::create("bg/World refference/Forest/Forest_background_1.png");
    bg->setPosition(visibleSize / 2);
    float scaleX = visibleSize.width / bg->getContentSize().width;
    float scaleY = visibleSize.height / bg->getContentSize().height;
    bg->setScale(MAX(scaleX, scaleY));
    setScene->addChild(bg, -1);

    // 使用AudioManager获取设置
    bool soundOn = AudioManager::isSoundEnabled();
    float bgmVol = AudioManager::getBGMVolume();
    float sfxVol = AudioManager::getSFXVolume();
    bool autoSaveEnabled = AudioManager::isAutoSaveEnabled();
    int autoSaveInterval = AudioManager::getAutoSaveInterval();

    // --- 开关 ---
    auto checkbox = ui::CheckBox::create("CloseNormal.png", "CloseSelected.png");
    checkbox->setPosition(Vec2(visibleSize.width * 0.3f, visibleSize.height * 0.7f));
    checkbox->setSelected(soundOn);
    setScene->addChild(checkbox);

    auto soundLabel = Label::createWithTTF("Sound", "fonts/Marker Felt.ttf", 36);
    soundLabel->setPosition(Vec2(checkbox->getPositionX() - 150, checkbox->getPositionY()));
    setScene->addChild(soundLabel);

    checkbox->addEventListener([=](Ref* sender, ui::CheckBox::EventType type) {
        bool on = (type == ui::CheckBox::EventType::SELECTED);
        // 使用AudioManager设置声音开关
        AudioManager::setSoundEnabled(on);
        });

    // --- BGM音量滑块 ---
    auto bgmLabel = Label::createWithTTF("BGM", "fonts/Marker Felt.ttf", 28);
    bgmLabel->setPosition(Vec2(checkbox->getPositionX() - 150, checkbox->getPositionY() - 60));
    setScene->addChild(bgmLabel);

    auto bgmSlider = ui::Slider::create();
    bgmSlider->loadBarTexture("Slider_Back.png");
    bgmSlider->loadSlidBallTextures("SliderNode_Normal.png", "SliderNode_Press.png", "SliderNode_Disable.png");
    bgmSlider->loadProgressBarTexture("Slider_PressBar.png");
    bgmSlider->setPosition(Vec2(checkbox->getPositionX() + 250, checkbox->getPositionY() -60));
    bgmSlider->setPercent(bgmVol * 100);
    setScene->addChild(bgmSlider);

    bgmSlider->addEventListener([=](Ref* sender, ui::Slider::EventType type) {
        if (type == ui::Slider::EventType::ON_PERCENTAGE_CHANGED)
        {
            float vol = bgmSlider->getPercent() / 100.0f;
            // 使用AudioManager设置BGM音量
            AudioManager::setBGMVolume(vol);
        }
        });

    // --- SFX音量滑块 ---
    auto sfxLabel = Label::createWithTTF("Player or NPC", "fonts/Marker Felt.ttf", 28);
    sfxLabel->setPosition(Vec2(checkbox->getPositionX() - 150, checkbox->getPositionY() - 120));
    setScene->addChild(sfxLabel);

    auto sfxSlider = ui::Slider::create();
    sfxSlider->loadBarTexture("Slider_Back.png");
    sfxSlider->loadSlidBallTextures("SliderNode_Normal.png", "SliderNode_Press.png", "SliderNode_Disable.png");
    sfxSlider->loadProgressBarTexture("Slider_PressBar.png");
    sfxSlider->setPosition(Vec2(checkbox->getPositionX() + 250, checkbox->getPositionY() - 120));
    sfxSlider->setPercent(sfxVol * 100);
    setScene->addChild(sfxSlider);

    sfxSlider->addEventListener([=](Ref* sender, ui::Slider::EventType type) {
        if (type == ui::Slider::EventType::ON_PERCENTAGE_CHANGED)
        {
            float vol = sfxSlider->getPercent() / 100.0f;
            // 使用AudioManager设置SFX音量
            AudioManager::setSFXVolume(vol);
        }
        });

    // --- 自动保存开关 ---
    auto autoSaveCheckbox = ui::CheckBox::create("CloseNormal.png", "CloseSelected.png");
    autoSaveCheckbox->setPosition(Vec2(checkbox->getPositionX(), checkbox->getPositionY() - 180));
    autoSaveCheckbox->setSelected(autoSaveEnabled);
    setScene->addChild(autoSaveCheckbox);

    auto autoSaveLabel = Label::createWithTTF("Auto Save", "fonts/Marker Felt.ttf", 36);
    autoSaveLabel->setPosition(Vec2(checkbox->getPositionX() - 120, autoSaveCheckbox->getPositionY()));
    setScene->addChild(autoSaveLabel);

    // --- 自动保存间隔滑块 ---
    auto intervalSlider = ui::Slider::create();
    intervalSlider->loadBarTexture("Slider_Back.png");
    intervalSlider->loadSlidBallTextures("SliderNode_Normal.png", "SliderNode_Press.png", "SliderNode_Disable.png");
    intervalSlider->loadProgressBarTexture("Slider_PressBar.png");
    intervalSlider->setPosition(Vec2(autoSaveCheckbox->getPositionX() + 180, autoSaveCheckbox->getPositionY() - 60));
    intervalSlider->setPercent((autoSaveInterval - 1) * 100 / 59); // 1-60分钟映射到0-100
    intervalSlider->setEnabled(autoSaveEnabled);
    intervalSlider->setBright(autoSaveEnabled);
    setScene->addChild(intervalSlider);

    autoSaveCheckbox->addEventListener([=](Ref* sender, ui::CheckBox::EventType type) {
        bool enabled = (type == ui::CheckBox::EventType::SELECTED);
        AudioManager::setAutoSaveEnabled(enabled);

        // 更新间隔滑块状态
        intervalSlider->setEnabled(enabled);
        intervalSlider->setBright(enabled);
        });

    // --- 自动保存间隔标签 ---
    auto intervalValueLabel = Label::createWithTTF(std::to_string(autoSaveInterval) + " min",
        "fonts/Marker Felt.ttf", 28);
    intervalValueLabel->setPosition(Vec2(autoSaveCheckbox->getPositionX() + 180, autoSaveCheckbox->getPositionY()));
    intervalValueLabel->setColor(autoSaveEnabled ? Color3B::YELLOW : Color3B::GRAY);
    intervalValueLabel->setName("IntervalValueLabel");
    setScene->addChild(intervalValueLabel);

    auto intervalLabel = Label::createWithTTF("Interval", "fonts/Marker Felt.ttf", 28);
    intervalLabel->setPosition(Vec2(checkbox->getPositionX() - 120, autoSaveCheckbox->getPositionY() - 60));
    setScene->addChild(intervalLabel);

    intervalSlider->addEventListener([=](Ref* sender, ui::Slider::EventType type) {
        if (type == ui::Slider::EventType::ON_PERCENTAGE_CHANGED)
        {
            int minutes = 1 + (intervalSlider->getPercent() * 59 / 100); // 映射回1-60
            AudioManager::setAutoSaveInterval(minutes);

            // 更新间隔标签
            if (intervalValueLabel) {
                intervalValueLabel->setString(std::to_string(minutes) + " min");
            }
        }
        });

    // 返回按钮
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 36);
    backLabel->setPosition(Vec2(100, visibleSize.height - 50));
    setScene->addChild(backLabel);

    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [=](Touch* t, Event* e) {
        if (backLabel->getBoundingBox().containsPoint(t->getLocation()))
        {
            Director::getInstance()->replaceScene(
                TransitionFade::create(0.5f, MainMenuScene::createScene()));
            return true;
        }
        return false;
        };
    setScene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, backLabel);
}

void MainMenuScene::menuLoadCallback(Ref* pSender)
{
    auto loadScene = Scene::create();
    Director::getInstance()->replaceScene(TransitionFade::create(1.0, loadScene));

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 背景
    auto bg = Sprite::create("bg/World refference/Forest/Forest_background_1.png");
    bg->setPosition(visibleSize / 2);
    auto size = bg->getContentSize();
    bg->setScale(visibleSize.width / size.width, visibleSize.height / size.height);
    loadScene->addChild(bg, -1);  // 背景永远最底层

    // 标题
    auto titleLabel = Label::createWithTTF("Load Game", "fonts/Marker Felt.ttf", 48);
    titleLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    titleLabel->setColor(Color3B::YELLOW);
    loadScene->addChild(titleLabel, 1);

    // 获取所有存档
    auto saveManager = SimpleSaveManager::getInstance();
    auto saves = saveManager->getAllSaves();

    // 创建6个存档槽位
    for (int i = 0; i < 6; i++) {
        int slotNumber = i + 1;

        // ---- 存档背景图 ----
        auto loadoption = Sprite::create("bg/loadoption.png");
        if (!loadoption) {
            loadoption = Sprite::create();
            loadoption->setColor(saves[i].isEmpty() ? Color3B::GRAY : Color3B(100, 100, 100));
            loadoption->setContentSize(Size(200, 150));
        }
        else {
            float scale = 0.4f;
            loadoption->setScale(scale);
        }

        // 计算位置 (2x3网格)
        int row = i / 2;      // 行：0-2
        int col = i % 2;      // 列：0-1

        float posX = visibleSize.width * (0.25f + col * 0.5f);
        float posY = visibleSize.height * 0.7f - row * 180;

        loadoption->setPosition(Vec2(posX, posY));
        loadoption->setName("Slot_" + std::to_string(slotNumber));
        loadScene->addChild(loadoption, 1);

        // 存档信息文字
        std::string slotText = saves[i].getDisplayText();
        auto loadLabel = Label::createWithTTF(slotText, "fonts/Marker Felt.ttf", 20);
        loadLabel->setPosition(loadoption->getPosition());
        loadLabel->setDimensions(180, 120);
        loadLabel->setHorizontalAlignment(TextHAlignment::CENTER);
        loadLabel->setVerticalAlignment(TextVAlignment::CENTER);
        loadLabel->setColor(saves[i].isEmpty() ? Color3B(150, 150, 150) : Color3B::WHITE);
        loadLabel->setName("SlotLabel_" + std::to_string(slotNumber));
        loadScene->addChild(loadLabel, 2);

        // 存档槽位点击事件
        auto slotListener = EventListenerTouchOneByOne::create();
        slotListener->setSwallowTouches(true);
        slotListener->onTouchBegan = [=](Touch* t, Event* e) {
            Rect bbox = loadoption->getBoundingBox();
            if (bbox.containsPoint(t->getLocation())) {
                if (saves[i].isEmpty()) {
                    // 空存档槽 - 创建新游戏
                    createNewGame(slotNumber);
                }
                else {
                    // 已有存档 - 加载游戏
                    loadGameFromSlot(slotNumber);
                }
                return true;
            }
            return false;
            };
        loadScene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(slotListener, loadoption);
    }

    // 返回按钮
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 36);
    backLabel->setPosition(Vec2(100, visibleSize.height - 50));
    loadScene->addChild(backLabel, 10);

    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [=](Touch* t, Event* e) {
        if (backLabel->getBoundingBox().containsPoint(t->getLocation()))
        {
            Director::getInstance()->replaceScene(
                TransitionFade::create(0.5f, MainMenuScene::createScene())
            );
            return true;
        }
        return false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, backLabel);
}

void MainMenuScene::createNewGame(int slotNumber) {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建初始存档数据
    SimpleSaveData newSave;
    newSave.slotNumber = slotNumber;
    newSave.saveName = "New Adventure";
    newSave.saveTime = time(nullptr);
    newSave.playerName = "Player";
    newSave.playerLevel = 1;
    newSave.playerExp = 0;
    newSave.playerPos.position.x = visibleSize.width / 2;
    newSave.playerPos.position.y = 200;

    // 保存存档
    if (SimpleSaveManager::getInstance()->saveGame(slotNumber, newSave)) {
        // 设置加载标记
        UserDefault::getInstance()->setIntegerForKey("load_slot", slotNumber);
        UserDefault::getInstance()->setBoolForKey("is_new_game", true);
        UserDefault::getInstance()->flush();

        // 开始新游戏
        auto gameScene = World::createScene();
        Director::getInstance()->replaceScene(
            TransitionFade::create(1.0f, gameScene)
        );
    }
}

void MainMenuScene::loadGameFromSlot(int slotNumber) {
    SimpleSaveData saveData;
    if (SimpleSaveManager::getInstance()->loadGame(slotNumber, saveData)) {
        // 设置加载标记
        UserDefault::getInstance()->setIntegerForKey("load_slot", slotNumber);
        UserDefault::getInstance()->setBoolForKey("is_new_game", false);
        UserDefault::getInstance()->flush();

        // 切换到游戏场景
        auto gameScene = World::createScene();
        Director::getInstance()->replaceScene(
            TransitionFade::create(1.0f, gameScene)
        );
    }
}

void MainMenuScene::menuCloseCallback(Ref* pSender)
{
    // 关闭游戏
    Director::getInstance()->end();
}
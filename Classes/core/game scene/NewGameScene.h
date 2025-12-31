#ifndef __NEW_GAME_SCENE_H__
#define __NEW_GAME_SCENE_H__

#include "cocos2d.h"
#include <systems/save/SaveData.h>

USING_NS_CC;

class NewGameScene : public Scene
{
public:
    static Scene* createScene();
    virtual bool init();
    void enableAutoSave(bool enable);
    void setAutoSaveIntervalMinutes(int minutes);
    void applyAutoSaveSettings();
    void autoSaveTick(float dt);
    CREATE_FUNC(NewGameScene);

private:
    // 玩家相关
    Sprite* player = nullptr;

    // 存档相关
    SimpleSaveData currentSaveData;
    int currentSaveSlot = 0;

    // 菜单状态管理
    bool isSettingEnabled = true;
    bool isSaveLoadMenuActive = false;

    // UI元素指针
    Label* settingLabel = nullptr;

    // 更新Setting按钮状态
    void updateSettingButtonState(bool enabled);

    // 新增：EnTT 相关
    entt::registry ecsRegistry;
    entt::entity playerEntity = entt::null;

    // 新增函数
    void initECS();
    void syncSpriteToECS();
    void syncECSToSprite();
    void createOrUpdatePlayerEntity();

    // 存档系统函数
    void initSaveSystem();
    void loadGameData();
    SimpleSaveData getCurrentGameState();
    void saveCurrentGame();
    void showSaveNotification(const std::string& message);

    // 存档/读档菜单函数
    void createSaveLoadMenu();
    void showSimpleSaveDialog(Scene* parentScene, int slotNumber);
    void showSaveLoadDialog(Scene* parentScene, int slotNumber, const SimpleSaveData& saveData);

    // 游戏功能函数
    void createPlayer();
};

#endif // __NEW_GAME_SCENE_H__
#include "AudioManager.h"

using namespace cocos2d;

// ========== 静态变量初始化 ==========
int   AudioManager::bgmId = -1;
float AudioManager::bgmVolume = 1.0f;
float AudioManager::sfxVolume = 1.0f;
bool  AudioManager::soundEnabled = true; // 添加声音开关状态
bool  AudioManager::autoSaveEnabled = true; // 默认开启自动保存
int   AudioManager::autoSaveInterval = 5;   // 默认5分钟自动保存

// ========== 加载设置（启动游戏时读一次） ==========
void AudioManager::loadSettings()
{
    auto ud = UserDefault::getInstance();
    bgmVolume = ud->getFloatForKey("global_bgm_volume", 1.0f);
    sfxVolume = ud->getFloatForKey("global_sfx_volume", 1.0f);
}


// ========== 保存设置 ==========
void AudioManager::saveSettings()
{
    auto ud = UserDefault::getInstance();
    ud->setFloatForKey("global_bgm_volume", bgmVolume);
    ud->setFloatForKey("global_sfx_volume", sfxVolume);
    ud->flush();
}

// ==================== 自动保存设置 ====================
void AudioManager::setAutoSaveEnabled(bool enabled)
{
    autoSaveEnabled = enabled;
    saveSettings();
}

bool AudioManager::isAutoSaveEnabled()
{
    return autoSaveEnabled;
}

void AudioManager::setAutoSaveInterval(int minutes)
{
    autoSaveInterval = minutes;
    if (autoSaveInterval < 1) autoSaveInterval = 1; // 最小1分钟
    if (autoSaveInterval > 60) autoSaveInterval = 60; // 最大60分钟
    saveSettings();
}

int AudioManager::getAutoSaveInterval()
{
    return autoSaveInterval;
}

// ===============================
//             BGM
// ===============================
void AudioManager::playBGM(const std::string& file)
{
    // 如果已经播放其他 BGM 先停掉
    if (bgmId != -1)
    {
        AudioEngine::stop(bgmId);
        bgmId = -1;
    }

    // 读取音量与开关设置
    float volume = UserDefault::getInstance()->getFloatForKey("sound_volume", 0.5f);
    bool enabled = UserDefault::getInstance()->getBoolForKey("sound_on", true);

    // 启动 BGM
    bgmId = AudioEngine::play2d(file, true, enabled ? volume : 0.0f);
}

void AudioManager::pauseBGM()
{
    if (bgmId != -1)
        AudioEngine::pause(bgmId);
}

void AudioManager::resumeBGM()
{
    if (bgmId != -1)
        AudioEngine::resume(bgmId);
}

void AudioManager::stopBGM()
{
    if (bgmId != -1)
    {
        AudioEngine::stop(bgmId);
        bgmId = -1;
    }
}

void AudioManager::setBGMVolume(float volume)
{
    bgmVolume = volume;
    if (bgmId != -1)
        AudioEngine::setVolume(bgmId, volume);

    saveSettings();
}

float AudioManager::getBGMVolume()
{
    return bgmVolume;
}

// 新增：设置声音开关
void AudioManager::setSoundEnabled(bool enabled)
{
    soundEnabled = enabled;
    if (bgmId != -1)
        AudioEngine::setVolume(bgmId, enabled ? bgmVolume : 0.0f);

    saveSettings();
}

// 新增：获取声音开关状态
bool AudioManager::isSoundEnabled()
{
    return soundEnabled;
}

void AudioManager::applySettings()
{
    bool enabled = UserDefault::getInstance()->getBoolForKey("sound_on", true);
    float volume = UserDefault::getInstance()->getFloatForKey("sound_volume", 0.5f);

    // 立即更新 BGM 音量
    if (bgmId != -1)
    {
        AudioEngine::setVolume(bgmId, enabled ? volume : 0.0f);
    }
}

// ===============================
//             SFX
// ===============================
void AudioManager::playSFX(const std::string& file)
{
    float sfxVolume = UserDefault::getInstance()->getFloatForKey("sfx_volume", 0.5f);
    bool enabled = UserDefault::getInstance()->getBoolForKey("sound_on", true);

    if (!enabled) return; // 关声音时 SFX 也不播放

    AudioEngine::play2d(file, false, sfxVolume);
}

void AudioManager::setSFXVolume(float volume)
{
    sfxVolume = volume;
    saveSettings();
}

float AudioManager::getSFXVolume()
{
    return sfxVolume;
}

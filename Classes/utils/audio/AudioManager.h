#pragma once
#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "audio/include/AudioEngine.h"
#include "cocos2d.h"

// 一个统一的音频管理类：负责 BGM 和 SFX
class AudioManager
{
public:

    // ======================
    // 背景音乐（BGM）管理
    // ======================

    // 播放 BGM（只保持一首 BGM 始终播放）,直接调用就行，这个默认满足设置调节的音量和开闭
    static void playBGM(const std::string& filePath);

    // 暂停 BGM
    static void pauseBGM();

    // 恢复 BGM
    static void resumeBGM();

    // 停止 BGM
    static void stopBGM();

    // 设置 BGM 音量
    static void setBGMVolume(float volume);

    // 获取 BGM 音量
    static float getBGMVolume();

    static void setSoundEnabled(bool enabled);

    static bool isSoundEnabled();

    // 应用当前用户设置（音量 + 开关）
    static void applySettings();

    // ======================
    // 音效（SFX）管理
    // ======================

    // 播放音效（一次性）
    static void playSFX(const std::string& filePath);

    // 设置全局 SFX 音量
    static void setSFXVolume(float volume);

    // 获取 SFX 音量
    static float getSFXVolume();

    // ======================
    // 自动保存设置
    // ======================
    static void setAutoSaveEnabled(bool enabled);
    static bool isAutoSaveEnabled();
    static void setAutoSaveInterval(int minutes);
    static int getAutoSaveInterval();

    // ======================
    // 设置保存 & 加载
    // ======================

    static void loadSettings();
    static void saveSettings();

    // 自动保存
    static bool autoSaveEnabled;
    static int autoSaveInterval; // 分钟

private:

    // BGM
    static int bgmId;
    static float bgmVolume;

    // SFX
    static float sfxVolume;

    static bool soundEnabled;

    
};

#endif

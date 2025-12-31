#ifndef __ECS_SYSTEM_ANIMATIONCONFIGLOADER_H__
#define __ECS_SYSTEM_ANIMATIONCONFIGLOADER_H__

#include "components/npc/AnimationStateComponent.h"
#include "cocos2d.h"
#include "json/document.h"
#include <string>
#include <memory>

namespace ecs {

/**
 * @brief 动画配置加载器 - 从JSON文件加载动画配置
 * 
 * JSON格式示例：
 * {
 *   "entity_type": "demon_eye",
 *   "default_state": "idle",
 *   "animations": {
 *     "idle": {
 *       "animation_set_id": "demon_eye_idle",
 *       "frame_sequence": [1, 2, 3, 2],
 *       "frame_time": 0.15,
 *       "loop": true,
 *       "priority": 0
 *     },
 *     "hovering": {
 *       "animation_set_id": "demon_eye_fly",
 *       "frame_sequence": [1, 2, 3, 4],
 *       "frame_time": 0.1,
 *       "loop": true,
 *       "priority": 1
 *     },
 *     "dashing": {
 *       "animation_set_id": "demon_eye_attack",
 *       "frame_sequence": [1, 2, 3, 4, 5],
 *       "frame_time": 0.08,
 *       "loop": false,
 *       "priority": 2
 *     }
 *   }
 * }
 */
class AnimationConfigLoader {
public:
    /**
     * @brief 从JSON文件加载动画配置
     * @param jsonFilePath JSON文件路径（相对于Resources目录）
     * @param outComponent 输出的AnimationStateComponent
     * @return 是否加载成功
     */
    static bool loadFromJson(const std::string& jsonFilePath, AnimationStateComponent& outComponent);
    
    /**
     * @brief 从JSON字符串加载动画配置
     * @param jsonString JSON字符串
     * @param outComponent 输出的AnimationStateComponent
     * @return 是否加载成功
     */
    static bool loadFromJsonString(const std::string& jsonString, AnimationStateComponent& outComponent);
    
private:
    /**
     * @brief 解析JSON文档到AnimationStateComponent
     */
    static bool parseJsonDocument(const rapidjson::Document& doc, AnimationStateComponent& outComponent);
    
    /**
     * @brief 解析单个动画状态数据
     */
    static bool parseAnimationStateData(const rapidjson::Value& animValue, AnimationStateData& outData);
};

} // namespace ecs

#endif // __ECS_SYSTEM_ANIMATIONCONFIGLOADER_H__

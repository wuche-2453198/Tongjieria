#include "AnimationConfigLoader.h"
#include "cocos2d.h"
#include "json/document.h"
#include "json/stringbuffer.h"

USING_NS_CC;

namespace ecs {

bool AnimationConfigLoader::loadFromJson(const std::string& jsonFilePath, AnimationStateComponent& outComponent) {
    // 读取JSON文件
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(jsonFilePath);
    std::string jsonContent = FileUtils::getInstance()->getStringFromFile(fullPath);
    
    if (jsonContent.empty()) {
        CCLOG("AnimationConfigLoader: Failed to read file: %s", jsonFilePath.c_str());
        return false;
    }
    
    return loadFromJsonString(jsonContent, outComponent);
}

bool AnimationConfigLoader::loadFromJsonString(const std::string& jsonString, AnimationStateComponent& outComponent) {
    // 解析JSON
    rapidjson::Document doc;
    doc.Parse(jsonString.c_str());
    
    if (doc.HasParseError()) {
        CCLOG("AnimationConfigLoader: JSON parse error at offset %u: %d",
              (unsigned)doc.GetErrorOffset(),
              doc.GetParseError());
        return false;
    }
    
    return parseJsonDocument(doc, outComponent);
}

bool AnimationConfigLoader::parseJsonDocument(const rapidjson::Document& doc, AnimationStateComponent& outComponent) {
    if (!doc.IsObject()) {
        CCLOG("AnimationConfigLoader: Root element must be an object");
        return false;
    }
    
    // 读取entity_type（可选，用于日志）
    std::string entityType = "unknown";
    if (doc.HasMember("entity_type") && doc["entity_type"].IsString()) {
        entityType = doc["entity_type"].GetString();
    }
    
    // 读取default_state
    if (doc.HasMember("default_state") && doc["default_state"].IsString()) {
        outComponent.defaultState = doc["default_state"].GetString();
        outComponent.currentState = outComponent.defaultState;
    }
    
    // 读取animations对象
    if (!doc.HasMember("animations") || !doc["animations"].IsObject()) {
        CCLOG("AnimationConfigLoader: Missing or invalid 'animations' object");
        return false;
    }
    
    const auto& animations = doc["animations"];
    
    // 遍历所有动画状态
    for (auto it = animations.MemberBegin(); it != animations.MemberEnd(); ++it) {
        std::string stateName = it->name.GetString();
        const auto& animValue = it->value;
        
        AnimationStateData animData;
        if (parseAnimationStateData(animValue, animData)) {
            outComponent.addStateAnimation(stateName, animData);
            CCLOG("AnimationConfigLoader: Loaded animation state '%s' for entity '%s'",
                  stateName.c_str(), entityType.c_str());
        } else {
            CCLOG("AnimationConfigLoader: Failed to parse animation state '%s'", stateName.c_str());
        }
    }
    
    if (outComponent.stateAnimations.empty()) {
        CCLOG("AnimationConfigLoader: No valid animations loaded");
        return false;
    }
    
    CCLOG("AnimationConfigLoader: Successfully loaded %zu animation states for entity '%s'",
          outComponent.stateAnimations.size(), entityType.c_str());
    
    return true;
}

bool AnimationConfigLoader::parseAnimationStateData(const rapidjson::Value& animValue, AnimationStateData& outData) {
    if (!animValue.IsObject()) {
        return false;
    }
    
    // 读取animation_set_id（必需）
    if (!animValue.HasMember("animation_set_id") || !animValue["animation_set_id"].IsString()) {
        CCLOG("AnimationConfigLoader: Missing 'animation_set_id'");
        return false;
    }
    outData.animationSetId = animValue["animation_set_id"].GetString();
    
    // 读取frame_sequence（必需）
    if (!animValue.HasMember("frame_sequence") || !animValue["frame_sequence"].IsArray()) {
        CCLOG("AnimationConfigLoader: Missing or invalid 'frame_sequence'");
        return false;
    }
    
    const auto& frameSeqArray = animValue["frame_sequence"];
    outData.frameSequence.clear();
    for (rapidjson::SizeType i = 0; i < frameSeqArray.Size(); ++i) {
        if (frameSeqArray[i].IsInt()) {
            outData.frameSequence.push_back(frameSeqArray[i].GetInt());
        }
    }
    
    if (outData.frameSequence.empty()) {
        CCLOG("AnimationConfigLoader: Empty frame_sequence");
        return false;
    }
    
    // 读取frame_time（可选，默认0.15）
    if (animValue.HasMember("frame_time") && animValue["frame_time"].IsNumber()) {
        outData.frameTime = animValue["frame_time"].GetFloat();
    }
    
    // 读取loop（可选，默认true）
    if (animValue.HasMember("loop") && animValue["loop"].IsBool()) {
        outData.loop = animValue["loop"].GetBool();
    }
    
    // 读取priority（可选，默认0）
    if (animValue.HasMember("priority") && animValue["priority"].IsInt()) {
        outData.priority = animValue["priority"].GetInt();
    }
    
    return true;
}

} // namespace ecs

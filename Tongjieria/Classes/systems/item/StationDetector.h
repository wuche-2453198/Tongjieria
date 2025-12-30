#ifndef __STATION_DETECTOR_H__
#define __STATION_DETECTOR_H__

#include "components/item/CraftingDef.h"
#include "cocos2d.h"
#include <set>

// 工作台检测器单例
// 管理当前可用的合成工作台
class StationDetector {
public:
    static StationDetector* getInstance();

    // 手动设置当前工作台（用于测试和简化实现）
    void setCurrentStations(const std::set<StationType>& stations);
    void addStation(StationType station);
    void removeStation(StationType station);
    void clear(); // 清除所有工作台（除了手工制作）

    // 查询当前环境
    const std::set<StationType>& getCurrentStations() const;
    bool hasStation(StationType station) const;

    // 事件分发
    void notifyStationChanged();

private:
    StationDetector();
    ~StationDetector() = default;

    static StationDetector* _instance;
    std::set<StationType> _currentStations; // 始终包含手工制作
};

#endif // __STATION_DETECTOR_H__

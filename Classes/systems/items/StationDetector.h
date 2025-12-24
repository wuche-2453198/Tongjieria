#ifndef __STATION_DETECTOR_H__
#define __STATION_DETECTOR_H__

#include "components/items/CraftingDef.h"
#include "cocos2d.h"
#include <set>

// Station detector singleton
// Manages currently available crafting stations
class StationDetector {
public:
    static StationDetector* getInstance();

    // Manually set current stations (for testing and simplified implementation)
    void setCurrentStations(const std::set<StationType>& stations);
    void addStation(StationType station);
    void removeStation(StationType station);
    void clear(); // Clear all stations except Hand

    // Query current environment
    const std::set<StationType>& getCurrentStations() const;
    bool hasStation(StationType station) const;

    // Event dispatch
    void notifyStationChanged();

private:
    StationDetector();
    ~StationDetector() = default;

    static StationDetector* _instance;
    std::set<StationType> _currentStations; // Always includes Hand
};

#endif // __STATION_DETECTOR_H__

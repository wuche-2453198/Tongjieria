#include "StationDetector.h"

using namespace cocos2d;

StationDetector* StationDetector::_instance = nullptr;

StationDetector::StationDetector() {
    // Default: always have Hand station available
    _currentStations.insert(StationType::Hand);
}

StationDetector* StationDetector::getInstance() {
    if (!_instance) {
        _instance = new StationDetector();
    }
    return _instance;
}

void StationDetector::setCurrentStations(const std::set<StationType>& stations) {
    _currentStations = stations;
    // Ensure Hand is always present
    _currentStations.insert(StationType::Hand);
    notifyStationChanged();
    CCLOG("StationDetector: Set stations, total: %zu", _currentStations.size());
}

void StationDetector::addStation(StationType station) {
    _currentStations.insert(station);
    notifyStationChanged();
    CCLOG("StationDetector: Added station %d, total: %zu",
          static_cast<int>(station), _currentStations.size());
}

void StationDetector::removeStation(StationType station) {
    if (station != StationType::Hand) { // Hand cannot be removed
        _currentStations.erase(station);
        notifyStationChanged();
        CCLOG("StationDetector: Removed station %d, total: %zu",
              static_cast<int>(station), _currentStations.size());
    }
}

void StationDetector::clear() {
    _currentStations.clear();
    _currentStations.insert(StationType::Hand);
    notifyStationChanged();
    CCLOG("StationDetector: Cleared stations (Hand only)");
}

const std::set<StationType>& StationDetector::getCurrentStations() const {
    return _currentStations;
}

bool StationDetector::hasStation(StationType station) const {
    return _currentStations.find(station) != _currentStations.end();
}

void StationDetector::notifyStationChanged() {
    Director::getInstance()->getEventDispatcher()
        ->dispatchCustomEvent("Event_StationChanged");
}

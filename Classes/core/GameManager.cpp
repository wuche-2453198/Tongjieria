#include "GameManager.h"

GameManager* GameManager::_instance = nullptr;

GameManager* GameManager::getInstance() {
    if (!_instance) {
        _instance = new GameManager();
    }
    return _instance;
}

void GameManager::reset() {
    _registry.clear();
    _playerEntity = entt::null;
}

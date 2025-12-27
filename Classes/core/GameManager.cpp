#include "GameManager.h"
#include "block_world.h"
#include "core/assets_manager.h"

GameManager* GameManager::_instance = nullptr;

GameManager* GameManager::getInstance() {
    if (!_instance) {
        _instance = new GameManager();
        // 先注册 AssetManager (BlockWorld 构造时需要从上下文获取它)
        _instance->_registry.ctx().emplace<AssetManager>();
        // 然后将 BlockWorld 直接构造在 registry 上下文中
        _instance->_registry.ctx().emplace<BlockWorld>(
            _instance->_registry,
            _instance->_dispatcher
        );
    }
    return _instance;
}

BlockWorld& GameManager::getBlockWorld() {
    return _registry.ctx().get<BlockWorld>();
}

const BlockWorld& GameManager::getBlockWorld() const {
    return _registry.ctx().get<BlockWorld>();
}

bool GameManager::hasBlockWorld() const {
    return _registry.ctx().contains<BlockWorld>();
}

void GameManager::reset() {
    _registry.clear();
    _playerEntity = entt::null;
    // BlockWorld 存储在 registry 上下文中,会随着 registry.clear() 一起清理
}

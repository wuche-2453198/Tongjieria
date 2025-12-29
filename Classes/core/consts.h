#pragma once
/**
* @brief 定义的方块的逻辑大小
*
* @note 应当尽量是二的整数倍
*/
constexpr static int BLOCK_SIZE = 16;
constexpr static int CHUNK_SIZE = 16;

/**
* @brief 方块类型枚举。
*/
enum class LayerType
{
    BLOCK, WALL
};

constexpr int BLOCK_Z_ORDER = 0;
constexpr int WALL_Z_ORDER = -1;
static int getZOrder(LayerType type) {return type == LayerType::BLOCK? BLOCK_Z_ORDER : WALL_Z_ORDER;}

/**
* @brief 渲染距离
*/
static unsigned int CHUNK_RENDER_START_DISTANCE = 10; 

/**
* @brief 渲染距离的截断距离
*/
static unsigned int CHUNK_RENDER_CUT_DISTANCE = 11;

/**
*/
enum WorldDirection
{
    UPPER,
    LOWER,
    LEFT,
    RIGHT,
    UPPERLEFT,
    UPPERRIGHT,
    LOWERLEFT,
    LOWERRIGHT
};
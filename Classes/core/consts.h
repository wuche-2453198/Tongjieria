#pragma once
/**
* 定义的方块的逻辑大小
*
* @note 应当尽量是二的整数倍
*/
constexpr int BLOCK_SIZE = 16;
constexpr static const int CHUNK_SIZE = 16;

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
#pragma once
/**
* @brief ����ķ�����߼���С
*
* @note Ӧ�������Ƕ���������
*/
constexpr static int BLOCK_SIZE = 24;
constexpr static int CHUNK_SIZE = 24;

/**
* @brief ��������ö�١�
*/
enum class LayerType
{
    BLOCK, WALL
};

constexpr int BLOCK_Z_ORDER = 0;
constexpr int WALL_Z_ORDER = -1;
static int getZOrder(LayerType type) {return type == LayerType::BLOCK? BLOCK_Z_ORDER : WALL_Z_ORDER;}

/**
* @brief ��Ⱦ����
*/
static unsigned int CHUNK_RENDER_START_DISTANCE = 4; 

/**
* @brief ��Ⱦ����ĽضϾ���
*/
static unsigned int CHUNK_RENDER_CUT_DISTANCE = 5;

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
#ifndef __ECS_COMPONENT_PLAYERTAG_H__
#define __ECS_COMPONENT_PLAYERTAG_H__

namespace ecs {

/**
 * @brief 玩家标记组件（Tag组件）
 */
struct PlayerTag
{
  char _dummy = 0; // EnTT需要非空结构体
};

} // namespace ecs

#endif // __ECS_COMPONENT_PLAYERTAG_H__

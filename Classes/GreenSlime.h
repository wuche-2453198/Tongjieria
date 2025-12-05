#ifndef __GREEN_SLIME_H__
#define __GREEN_SLIME_H__

#include "SlimeEnemy.h"

/**
 * @class GreenSlime
 * @brief 绿色史莱姆 - SlimeEnemy的具体实现
 *
 * 使用绿色史莱姆贴图,跳跃间隔为2.5秒
 */
class GreenSlime : public SlimeEnemy {
public:
  /**
   * 创建绿色史莱姆实例
   */
  static GreenSlime *create();

  /**
   * 初始化绿色史莱姆
   */
  virtual bool init() override;

protected:
  GreenSlime();
  virtual ~GreenSlime() = default;
};

#endif // __GREEN_SLIME_H__

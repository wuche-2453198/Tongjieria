#ifndef __BLUE_SLIME_H__
#define __BLUE_SLIME_H__

#include "SlimeEnemy.h"

class BlueSlime : public SlimeEnemy {
public:
  BlueSlime();
  virtual bool init() override;
  static BlueSlime *create();
};

#endif // __BLUE_SLIME_H__

#ifndef __YELLOW_SLIME_H__
#define __YELLOW_SLIME_H__

#include "SlimeEnemy.h"

class YellowSlime : public SlimeEnemy {
public:
	YellowSlime();
	virtual bool init() override;
	static YellowSlime* create();
};

#endif // __BLUE_SLIME_H__

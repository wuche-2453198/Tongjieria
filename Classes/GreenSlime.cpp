#include "GreenSlime.h"

GreenSlime::GreenSlime() {}

GreenSlime *GreenSlime::create() {
  GreenSlime *slime = new (std::nothrow) GreenSlime();
  if (slime && slime->init()) {
    slime->autorelease();
    return slime;
  }
  CC_SAFE_DELETE(slime);
  return nullptr;
}

bool GreenSlime::init() {
  // 调用父类SlimeEnemy的init，传入"Green"作为颜色参数
  if (!SlimeEnemy::init("Green")) {
    return false;
  }

  // 设置缩放 - 调小到1倍
  this->setScale(1.0f);

  // 设置绿史莱姆特有的跳跃冷却时间为2.5秒
  _jumpCooldown = 2.5f;
  // 初始化时设置冷却计时器，防止刚落地就跳跃
  _jumpCooldownTimer = _jumpCooldown;

  return true;
}

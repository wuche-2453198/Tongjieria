#include "YellowSlime.h"

YellowSlime::YellowSlime() {}
YellowSlime *YellowSlime::create() {
  YellowSlime *slime = new (std::nothrow) YellowSlime();
  if (slime && slime->init()) {
    slime->autorelease();
    return slime;
  }
  CC_SAFE_DELETE(slime);
  return nullptr;
}

bool YellowSlime::init() {
  // 调用父类SlimeEnemy的init，传入"Yellow"作为颜色参数
  if (!SlimeEnemy::init("Yellow")) {
    return false;
  }

  // 设置缩放 - 调小到1.2倍
  this->setScale(1.2f);

  // 设置标签为2，用于识别黄色史莱姆
  this->setTag(2);

  // 设置黄史莱姆特有的跳跃冷却时间为2.0秒
  _jumpCooldown = 2.0f;
  // 初始化时设置冷却计时器，防止刚落地就跳跃
  _jumpCooldownTimer = _jumpCooldown;

  // 设置黄史莱姆的最大跳跃力度为绿史莱姆的1.2倍
  // 绿史莱姆默认：水平300，垂直500
  _maxHorizontalImpulse = 300.0f * 1.2f; // 360
  _maxVerticalImpulse = 500.0f * 1.2f;   // 600

  // 清除父类的默认掉落物，添加黄史莱姆特有的掉落物
  _dropTable.clear();
  addDropItem("gel", 3, 5, 0.8f);    // 80%概率掉落3-5个凝胶（比蓝史莱姆多）
  addDropItem("coin", 10, 25, 0.5f); // 50%概率掉落10-25金币（比蓝史莱姆多）

  return true;
}
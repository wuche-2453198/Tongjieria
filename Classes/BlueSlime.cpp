#include "BlueSlime.h"

BlueSlime::BlueSlime() {}

BlueSlime *BlueSlime::create() {
  BlueSlime *slime = new (std::nothrow) BlueSlime();
  if (slime && slime->init()) {
    slime->autorelease();
    return slime;
  }
  CC_SAFE_DELETE(slime);
  return nullptr;
}

bool BlueSlime::init() {
  // 调用父类SlimeEnemy的init，传入"Blue"作为颜色参数
  if (!SlimeEnemy::init("Blue")) {
    return false;
  }

  // 设置缩放 - 调小到1.2倍
  this->setScale(1.2f);

  // 设置标签为1，用于识别蓝色史莱姆
  this->setTag(1);

  // 设置蓝史莱姆特有的跳跃冷却时间为2.2秒
  _jumpCooldown = 2.2f;
  // 初始化时设置冷却计时器，防止刚落地就跳跃
  _jumpCooldownTimer = _jumpCooldown;

  // 设置蓝史莱姆的最大跳跃力度为绿史莱姆的1.1倍
  // 绿史莱姆默认：水平300，垂直500
  _maxHorizontalImpulse = 300.0f * 1.1f; // 330
  _maxVerticalImpulse = 500.0f * 1.1f;   // 550

  // 清除父类的默认掉落物，添加蓝史莱姆特有的掉落物
  _dropTable.clear();
  addDropItem("gel", 2, 4, 0.8f);   // 80%概率掉落2-4个凝胶（比绿史莱姆多）
  addDropItem("coin", 8, 20, 0.5f); // 50%概率掉落8-20金币（比绿史莱姆多）

  return true;
}
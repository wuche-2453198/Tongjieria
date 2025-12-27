#include "EventSystem.h"

namespace ecs {

// EventSystem 是一个模板密集型的头文件类
// 大部分实现都在头文件中作为模板
// 此 cpp 文件用于：
// 1. 确保单例的正确链接
// 2. 未来可能的非模板方法实现

// 注意：由于 EventSystem 使用单例模式且大部分是模板方法，
// 实际的实现都在头文件中。此文件主要用于编译单元的完整性。

} // namespace ecs

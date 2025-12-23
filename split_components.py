#!/usr/bin/env python3
"""
自动拆分Components.h和SystemsEntt.h到独立文件
"""

import re
import os

# 组件定义列表（从Components.h提取）
COMPONENTS = [
    "TransformComponent",
    "SlimeSpriteComponent",
    "HealthComponent",
    "PlayerTag",
    "EnemyTag",
    "LootComponent",
    "LifetimeComponent",
    "CombatComponent",
    "AggroComponent",
    "GroundDetectorComponent",
    "JumpMovementComponent",
    "MonsterSpriteComponent",
    "WarriorMovementComponent",
    "DeathSpawnComponent",
    "SlowFallComponent",
    "ProjectileAttackComponent",
    "ProjectileComponent",
    "DebuffComponent",
    "ProjectileSpriteComponent",
    "DemonEyeMovementComponent",
    "KingSlimeComponent",
]

# 系统定义列表（从SystemsEntt.h提取）
SYSTEMS = [
    "ISystemEntt",
    "HealthSystemEntt",
    "CombatSystemEntt",
    "LifetimeSystemEntt",
    "MonsterAnimationSystemEntt",
    "SlimeRenderSystemEntt",
    "DebuffSystemEntt",
    "SlimeSyncSystemEntt",
    "MonsterSyncSystemEntt",
    "GroundDetectorSystemEntt",
    "MonsterGroundDetectorSystemEntt",
    "SlowFallSystemEntt",
    "AggroSystemEntt",
    "JumpMovementSystemEntt",
    "WarriorAISystemEntt",
    "ProjectileSystemEntt",
    "ProjectileAttackSystemEntt",
    "DemonEyeAISystemEntt",
    "KingSlimeAISystemEntt",
    "ProjectileCollisionSystemEntt",
]

def extract_component_code(content, component_name):
    """提取组件代码（包含注释）"""
    # 查找组件开始位置（从注释开始）
    pattern = rf'(\/\*\*[\s\S]*?\*\/\s*struct\s+{component_name}\s*{{[\s\S]*?}};)'
    match = re.search(pattern, content)
    
    if not match:
        # 尝试不带注释的模式
        pattern = rf'(struct\s+{component_name}\s*{{[\s\S]*?}};)'
        match = re.search(pattern, content)
    
    if match:
        return match.group(1)
    return None

def extract_system_code(content, system_name):
    """提取系统代码（包含注释）"""
    # 查找系统开始位置
    pattern = rf'(\/\*\*[\s\S]*?\*\/\s*class\s+{system_name}\s*[:\s\w]*{{[\s\S]*?^}};)'
    match = re.search(pattern, content, re.MULTILINE)
    
    if not match:
        # 尝试不带注释的模式
        pattern = rf'(class\s+{system_name}\s*[:\s\w]*{{[\s\S]*?^}};)'
        match = re.search(pattern, content, re.MULTILINE)
    
    if match:
        return match.group(1)
    return None

def create_component_file(component_name, code, output_dir):
    """创建独立组件文件"""
    filename = f"{component_name}.h"
    filepath = os.path.join(output_dir, filename)
    
    header_guard = f"__ECS_COMPONENT_{component_name.upper()}_H__"
    
    content = f"""#ifndef {header_guard}
#define {header_guard}

#include "../Entity.h"
#include "cocos2d.h"
#include <functional>
#include <string>
#include <vector>

namespace ecs {{

{code}

}} // namespace ecs

#endif // {header_guard}
"""
    
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"Created: {filename}")

def create_system_file(system_name, code, output_dir):
    """创建独立系统文件"""
    filename = f"{system_name}.h"
    filepath = os.path.join(output_dir, filename)
    
    header_guard = f"__ECS_SYSTEM_{system_name.upper()}_H__"
    
    content = f"""#ifndef {header_guard}
#define {header_guard}

#include <entt/entt.hpp>
#include "../Components.h"
#include "../SpriteComponent.h"
#include <vector>
#include <memory>
#include <algorithm>

namespace ecs {{

{code}

}} // namespace ecs

#endif // {header_guard}
"""
    
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"Created: {filename}")

def create_aggregate_header(items, item_type, output_dir, parent_dir):
    """创建聚合头文件"""
    if item_type == "component":
        filename = "AllComponents.h"
        header_guard = "__ECS_ALL_COMPONENTS_H__"
        folder = "components"
    else:
        filename = "AllSystems.h"
        header_guard = "__ECS_ALL_SYSTEMS_H__"
        folder = "systems"
    
    filepath = os.path.join(parent_dir, filename)
    
    includes = "\n".join([f'#include "{folder}/{item}.h"' for item in items])
    
    content = f"""#ifndef {header_guard}
#define {header_guard}

/**
 * @file {filename}
 * @brief 聚合所有{'组件' if item_type == 'component' else '系统'}的便捷头文件
 * 
 * 使用方法：
 * #include "ecs/{filename}"
 */

{includes}

#endif // {header_guard}
"""
    
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"Created aggregate header: {filename}")

def main():
    base_dir = r"D:\code\c++\Paradigm Homework\tongjieriar\Tongjieria\Classes\ecs"
    components_dir = os.path.join(base_dir, "components")
    systems_dir = os.path.join(base_dir, "systems")
    
    # 确保目录存在
    os.makedirs(components_dir, exist_ok=True)
    os.makedirs(systems_dir, exist_ok=True)
    
    # 读取源文件
    components_file = os.path.join(base_dir, "Components.h")
    systems_file = os.path.join(base_dir, "SystemsEntt.h")
    
    with open(components_file, 'r', encoding='utf-8') as f:
        components_content = f.read()
    
    with open(systems_file, 'r', encoding='utf-8') as f:
        systems_content = f.read()
    
    # 提取并创建组件文件
    print("\n=== 拆分组件 ===")
    for comp in COMPONENTS:
        code = extract_component_code(components_content, comp)
        if code:
            create_component_file(comp, code, components_dir)
        else:
            print(f"WARNING: Could not extract {comp}")
    
    # 提取并创建系统文件
    print("\n=== 拆分系统 ===")
    for sys in SYSTEMS:
        code = extract_system_code(systems_content, sys)
        if code:
            create_system_file(sys, code, systems_dir)
        else:
            print(f"WARNING: Could not extract {sys}")
    
    # 创建聚合头文件
    print("\n=== 创建聚合头文件 ===")
    create_aggregate_header(COMPONENTS, "component", components_dir, base_dir)
    create_aggregate_header(SYSTEMS, "system", systems_dir, base_dir)
    
    print("\n✅ 拆分完成！")
    print(f"组件文件: {components_dir}")
    print(f"系统文件: {systems_dir}")

if __name__ == "__main__":
    main()

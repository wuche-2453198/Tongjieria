#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
批量转换 GIF 文件为 PNG 格式
用于修复 Cocos2d 无法加载 GIF 格式文件的问题
"""

import os
from PIL import Image

def convert_gif_to_png(directory):
    """转换指定目录中所有伪装成 PNG 的 GIF 文件"""

    if not os.path.exists(directory):
        print(f"错误: 目录不存在: {directory}")
        return

    print(f"开始处理目录: {directory}")
    converted_count = 0
    error_count = 0

    # 遍历目录中的所有 .png 文件
    for filename in os.listdir(directory):
        if not filename.endswith('.png'):
            continue

        filepath = os.path.join(directory, filename)

        try:
            # 打开图片
            img = Image.open(filepath)

            # 检查是否是 GIF 格式
            if img.format == 'GIF':
                print(f"Found GIF file: {filename}")

                # 转换为 RGBA 模式（PNG 支持透明度）
                if img.mode != 'RGBA':
                    img = img.convert('RGBA')

                # 保存为真正的 PNG 格式（覆盖原文件）
                img.save(filepath, 'PNG')
                print(f"  [OK] Converted: {filename}")
                converted_count += 1
            else:
                print(f"Skip (already PNG): {filename}")

        except Exception as e:
            print(f"  [FAIL] Error: {filename} - {str(e)}")
            error_count += 1

    print(f"\nConversion complete!")
    print(f"  Success: {converted_count} files")
    print(f"  Failed: {error_count} files")

if __name__ == "__main__":
    # 转换 walk 文件夹
    walk_dir = r"D:\test\Terraria_tj\Resources\player\walk"
    convert_gif_to_png(walk_dir)

    print("\n如果需要转换其他文件夹，可以取消注释下面的代码:")
    print("# convert_gif_to_png(r'D:\\test\\Terraria_tj\\Resources\\player\\其他文件夹')")

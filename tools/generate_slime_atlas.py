#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Slime Texture Atlas Generator (Python 2.7 compatible)
Merges all slime PNG images into a single atlas and generates Cocos2d-x compatible plist file

Usage:
    python generate_slime_atlas.py

Output:
    Resources/atlas/slimes_atlas.png  - Merged texture atlas
    Resources/atlas/slimes_atlas.plist - Cocos2d-x sprite frame description file
"""

from __future__ import print_function
import os
import math

try:
    from PIL import Image
except ImportError:
    print("ERROR: PIL/Pillow not installed. Run: pip install Pillow")
    exit(1)

# Configuration
RESOURCES_DIR = "../Resources"
OUTPUT_DIR = "../Resources/atlas"
SLIME_DIRS = [
    "picture/Minor_monster/GreenSlime",
    "picture/Minor_monster/BlueSlime",
    "picture/Minor_monster/RedSlime",
    "picture/Minor_monster/YellowSlime",
    "picture/Minor_monster/PurpleSlime",
    "picture/Minor_monster/PinkSlime",
    "picture/Minor_monster/IceSlime",
    "picture/Minor_monster/BlackSlime",
    "picture/Minor_monster/JungleSlime",
    "picture/Minor_monster/BabySlime",
    "picture/Minor_monster/MotherSlime",
    "picture/Minor_monster/UmbrellaSlime",
    "picture/Minor_monster/Spiked_Slime",
    "picture/Minor_monster/Spiked_IceSlime",
    "picture/Minor_monster/Spiked_JungleSlime",
]

ATLAS_NAME = "slimes_atlas"
PADDING = 2  # Spacing between images to prevent texture bleeding


def collect_images(base_dir, subdirs):
    """Collect all PNG images"""
    images = []
    for subdir in subdirs:
        full_path = os.path.join(base_dir, subdir)
        if not os.path.exists(full_path):
            print("Warning: Directory not found: " + full_path)
            continue
        
        for filename in sorted(os.listdir(full_path)):
            if filename.endswith('.png'):
                img_path = os.path.join(full_path, filename)
                # Generate frame name: dirname_filename (without extension)
                dir_name = os.path.basename(subdir)
                frame_name = dir_name + "_" + os.path.splitext(filename)[0]
                images.append({
                    'path': img_path,
                    'name': frame_name,
                    'filename': filename
                })
    return images


def calculate_atlas_size(images, padding):
    """Calculate atlas dimensions (using simple row arrangement algorithm)"""
    if not images:
        return 0, 0
    
    # Load all images to get dimensions
    for img_info in images:
        img = Image.open(img_info['path'])
        img_info['width'] = img.width
        img_info['height'] = img.height
        img.close()
    
    # Sort by height (descending)
    images.sort(key=lambda x: x['height'], reverse=True)
    
    # Calculate total area, estimate atlas size
    total_area = sum((img['width'] + padding) * (img['height'] + padding) for img in images)
    side = int(math.sqrt(total_area) * 1.2)  # 20% margin
    
    # Ensure power of 2 (OpenGL optimization)
    atlas_size = 1
    while atlas_size < side:
        atlas_size *= 2
    
    # Max limit 4096
    atlas_size = min(atlas_size, 4096)
    
    return atlas_size, atlas_size


def pack_images(images, atlas_width, atlas_height, padding):
    """Simple row-based packing algorithm"""
    x, y = padding, padding
    row_height = 0
    
    for img_info in images:
        w, h = img_info['width'], img_info['height']
        
        # Check if need to wrap to next row
        if x + w + padding > atlas_width:
            x = padding
            y += row_height + padding
            row_height = 0
        
        # Check if exceeds height
        if y + h + padding > atlas_height:
            print("Warning: Atlas too small for all images!")
            break
        
        img_info['x'] = x
        img_info['y'] = y
        
        x += w + padding
        row_height = max(row_height, h)
    
    return images


def create_atlas(images, atlas_width, atlas_height):
    """Create atlas image"""
    atlas = Image.new('RGBA', (atlas_width, atlas_height), (0, 0, 0, 0))
    
    for img_info in images:
        if 'x' not in img_info:
            continue
        
        img = Image.open(img_info['path'])
        atlas.paste(img, (img_info['x'], img_info['y']))
        img.close()
    
    return atlas


def generate_plist(images, atlas_width, atlas_height, texture_filename):
    """Generate Cocos2d-x compatible plist file (XML format)"""
    frames = []
    
    for img_info in images:
        if 'x' not in img_info:
            continue
        
        frame = '''    <key>{name}</key>
    <dict>
        <key>frame</key>
        <string>{{{{{x},{y}}},{{{w},{h}}}}}</string>
        <key>offset</key>
        <string>{{0,0}}</string>
        <key>rotated</key>
        <false/>
        <key>sourceColorRect</key>
        <string>{{{{0,0}},{{{w},{h}}}}}</string>
        <key>sourceSize</key>
        <string>{{{w},{h}}}</string>
    </dict>'''.format(
            name=img_info['name'],
            x=img_info['x'],
            y=img_info['y'],
            w=img_info['width'],
            h=img_info['height']
        )
        frames.append(frame)
    
    plist_content = '''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>frames</key>
    <dict>
{frames}
    </dict>
    <key>metadata</key>
    <dict>
        <key>format</key>
        <integer>2</integer>
        <key>realTextureFileName</key>
        <string>{tex}</string>
        <key>size</key>
        <string>{{{aw},{ah}}}</string>
        <key>textureFileName</key>
        <string>{tex}</string>
    </dict>
</dict>
</plist>'''.format(
        frames='\n'.join(frames),
        tex=texture_filename,
        aw=atlas_width,
        ah=atlas_height
    )
    
    return plist_content


def main():
    # Ensure output directory exists
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)
    
    # Collect images
    print("Collecting slime images...")
    images = collect_images(RESOURCES_DIR, SLIME_DIRS)
    print("Found " + str(len(images)) + " images")
    
    if not images:
        print("No images found!")
        return
    
    # Calculate atlas size
    print("Calculating atlas size...")
    atlas_width, atlas_height = calculate_atlas_size(images, PADDING)
    print("Atlas size: " + str(atlas_width) + "x" + str(atlas_height))
    
    # Pack images
    print("Packing images...")
    images = pack_images(images, atlas_width, atlas_height, PADDING)
    
    # Create atlas
    print("Creating atlas image...")
    atlas = create_atlas(images, atlas_width, atlas_height)
    
    # Save atlas
    atlas_path = os.path.join(OUTPUT_DIR, ATLAS_NAME + ".png")
    atlas.save(atlas_path, "PNG")
    print("Saved atlas: " + atlas_path)
    
    # Generate plist
    print("Generating plist...")
    plist_content = generate_plist(images, atlas_width, atlas_height, ATLAS_NAME + ".png")
    plist_path = os.path.join(OUTPUT_DIR, ATLAS_NAME + ".plist")
    with open(plist_path, 'w') as f:
        f.write(plist_content)
    print("Saved plist: " + plist_path)
    
    # Output frame name mapping (for code reference)
    print("")
    print("=== Frame Names ===")
    count = 0
    for img_info in images:
        if 'x' in img_info:
            if count < 10:
                print("  " + img_info['name'])
            count += 1
    if count > 10:
        print("  ... and " + str(count - 10) + " more")
    
    packed_count = len([i for i in images if 'x' in i])
    print("")
    print("Done! Atlas contains " + str(packed_count) + " frames")
    print("Expected Draw Calls reduction: ~" + str(len(images)) + " -> 1")


if __name__ == "__main__":
    main()

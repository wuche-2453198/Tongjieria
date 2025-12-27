# -*- coding: utf-8 -*-
"""
GIF to PNG frames converter
Convert GIF files in current directory to PNG sequence frames
Naming format: {gif_name}1.png, {gif_name}2.png, ...
"""

import os
import sys
import glob

try:
    from PIL import Image
except ImportError:
    print("Please install Pillow: pip install Pillow")
    sys.exit(1)


def gif_to_png_frames(gif_path):
    """Convert GIF file to PNG sequence frames"""
    if not os.path.exists(gif_path):
        print("File not found: " + gif_path)
        return
    
    base_name = os.path.splitext(os.path.basename(gif_path))[0]
    output_dir = os.path.dirname(gif_path)
    
    try:
        img = Image.open(gif_path)
        frame_count = 0
        while True:
            try:
                img.seek(frame_count)
                frame_num = frame_count + 1
                output_path = os.path.join(output_dir, base_name + str(frame_num) + ".png")
                
                frame = img.convert("RGBA")
                frame.save(output_path, "PNG")
                print("  Generated: " + os.path.basename(output_path))
                
                frame_count += 1
            except EOFError:
                break
        
        img.close()
        print("Done! Generated " + str(frame_count) + " frames")
            
    except Exception as e:
        print("Error processing GIF: " + str(e))


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    gif_files = glob.glob(os.path.join(script_dir, "*.gif"))
    
    if not gif_files:
        print("No GIF files found in: " + script_dir)
        return
    
    print("Found " + str(len(gif_files)) + " GIF file(s)")
    print("-" * 40)
    
    for gif_file in gif_files:
        print("\nProcessing: " + os.path.basename(gif_file))
        gif_to_png_frames(gif_file)


if __name__ == "__main__":
    main()

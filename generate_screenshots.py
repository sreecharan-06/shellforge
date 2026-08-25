import os
import subprocess
import shutil
from PIL import Image, ImageDraw, ImageFont

scratch_dir = 'C:/Users/saich/.gemini/antigravity/scratch/shellforge'
dest_dir = 'C:/Users/saich/.gemini/antigravity/brain/429606e2-9ff0-49da-8ea0-683cfa9ca2f0'
os.makedirs(dest_dir, exist_ok=True)

# Helper to run shellforge inside WSL with input and return output
def run_shellforge(input_str):
    input_file = os.path.join(scratch_dir, 'temp_in.txt')
    output_file = os.path.join(scratch_dir, 'temp_out.txt')
    
    with open(input_file, 'w') as f:
        f.write(input_str)
        
    # Run in WSL
    subprocess.run(['wsl', 'rm', '-rf', 'testdir'], cwd=scratch_dir)
    subprocess.run('wsl bash -c "./shellforge < temp_in.txt > temp_out.txt 2>&1"', shell=True, cwd=scratch_dir)
    
    with open(output_file, 'r', errors='ignore') as f:
        out = f.read()
        
    # Clean up temp files
    try:
        os.remove(input_file)
        os.remove(output_file)
    except:
        pass
        
    return out

# Helper to compile using WSL make
def compile_shellforge():
    subprocess.run(['wsl', 'make', 'clean'], cwd=scratch_dir)
    subprocess.run(['wsl', 'make'], cwd=scratch_dir)

# Helper to draw terminal text to a PNG
def draw_terminal(text, filename):
    lines = text.split('\n')
    
    # Setup styling
    bg_color = (30, 30, 30)      # #1E1E1E
    text_color = (212, 212, 212) # #D4D4D4
    prompt_color = (156, 220, 254) # Light Blue
    banner_color = (206, 145, 120) # Peach/Coral
    
    # Try loading Consolas font, otherwise fallback to default
    font_size = 15
    try:
        font = ImageFont.truetype('C:/Windows/Fonts/consola.ttf', font_size)
    except:
        font = ImageFont.load_default()
        
    # Calculate image height dynamically
    line_height = font_size + 5
    width = 850
    height = max(400, len(lines) * line_height + 40)
    
    # Create image
    img = Image.new('RGB', (width, height), bg_color)
    draw = ImageDraw.Draw(img)
    
    # Draw title bar
    draw.rectangle([0, 0, width, 30], fill=(45, 45, 45))
    draw.text((15, 7), "ShellForge Terminal - " + filename, fill=(150, 150, 150), font=font)
    
    # Draw terminal buttons (decorations)
    draw.ellipse([width - 25, 9, width - 15, 19], fill=(255, 96, 92))   # Red
    draw.ellipse([width - 45, 9, width - 35, 19], fill=(255, 189, 46))  # Yellow
    draw.ellipse([width - 65, 9, width - 55, 19], fill=(39, 201, 63))   # Green
    
    y = 40
    for line in lines:
        if line.startswith('shellforge$ '):
            # Draw prompt separately to colorize it
            draw.text((15, y), "shellforge$ ", fill=prompt_color, font=font)
            cmd_text = line[len('shellforge$ '):]
            draw.text((15 + draw.textlength("shellforge$ ", font=font), y), cmd_text, fill=text_color, font=font)
        elif '===' in line or '---' in line:
            draw.text((15, y), line, fill=banner_color, font=font)
        else:
            draw.text((15, y), line, fill=text_color, font=font)
        y += line_height
        
    img_path = os.path.join(dest_dir, filename)
    img.save(img_path)
    print(f"Generated screenshot: {img_path}")

def main():
    main_c_path = os.path.join(scratch_dir, 'src/main.c')
    
    # 1. GENERATE MILESTONE 2.2 SCREENSHOT (with parser & expand prints)
    # Read original main.c
    with open(main_c_path, 'r') as f:
        orig_main = f.read()
        
    # Modify main.c to uncomment token_print and pipeline_print
    m22_main = orig_main.replace('// token_print(&tokens);', 'token_print(&tokens);')
    # Or in our main.c we didn't have comments, let's insert them!
    # Let's insert them right after lexer and parser
    m22_main = orig_main.replace(
        'lexer(line, &tokens);',
        'lexer(line, &tokens);\n        token_print(&tokens);'
    ).replace(
        'expand_variables(&pipeline);',
        'expand_variables(&pipeline);\n            pipeline_print(&pipeline);'
    )
    
    with open(main_c_path, 'w') as f:
        f.write(m22_main)
        
    compile_shellforge()
    
    m22_input = """echo "Welcome to Milestone 2"
echo $USER
exit
"""
    m22_out = run_shellforge(m22_input)
    draw_terminal(m22_out, 'screenshot_m22.png')
    
    # Restore original main.c
    with open(main_c_path, 'w') as f:
        f.write(orig_main)
        
    compile_shellforge()
    
    # 2. GENERATE MILESTONE 3.1 SCREENSHOT (builtins)
    m31_input = """pwd
mkdir testdir
cd testdir
pwd
cd ..
pwd
cd ~
pwd
echo Hello World
exit
"""
    m31_out = run_shellforge(m31_input)
    draw_terminal(m31_out, 'screenshot_m31.png')
    
    # 3. GENERATE MILESTONE 3.2 SCREENSHOT (externals & redirection)
    m32_input = """pwd
mkdir testdir
cd testdir
echo hello > test.txt
cat test.txt
echo world >> test.txt
cat test.txt
date
cd ..
rm -rf testdir
nonexistentcommand
cd /does/not/exist
cd a b
exit
"""
    m32_out = run_shellforge(m32_input)
    draw_terminal(m32_out, 'screenshot_m32.png')

if __name__ == '__main__':
    main()

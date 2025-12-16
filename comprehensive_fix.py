#!/usr/bin/env python3
import os
import re
import glob

def comprehensive_fix():
    """彻底修复所有编码问题"""
    
    cpp_files = glob.glob('src/*.cpp')
    
    for file_path in cpp_files:
        print(f"处理文件: {file_path}")
        
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # 修复所有中文字符串，添加u8前缀
        # 处理所有包含中文字符的字符串字面量
        
        def fix_chinese_strings(text):
            lines = text.split('\n')
            fixed_lines = []
            
            for line in lines:
                # 跳过预处理指令和注释
                stripped = line.strip()
                if stripped.startswith('#') or stripped.startswith('//'):
                    fixed_lines.append(line)
                    continue
                
                # 处理包含中文字符的字符串
                # 匹配模式: 任何字符然后中文然后任何字符的字符串
                def fix_string(match):
                    full_match = match.group(0)
                    # 如果已经包含u8前缀，跳过
                    if 'u8"' in full_match:
                        return full_match
                    
                    # 添加u8前缀
                    return full_match.replace('"', 'u8"', 1)
                
                # 查找并修复所有包含中文的字符串
                # 更精确的匹配模式
                pattern = r'(?<![u8])"([^"]*[一-龯][^"]*)"'
                line = re.sub(pattern, fix_string, line)
                
                # 处理std::cout << "中文"的情况
                pattern2 = r'(std::cout << )"([^"]*[一-龯][^"]*)"'
                line = re.sub(pattern2, r'\1u8"\2"', line)
                
                fixed_lines.append(line)
            
            return '\n'.join(fixed_lines)
        
        new_content = fix_chinese_strings(content)
        
        # 保存修改后的文件
        if new_content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(new_content)
            print(f"  已修复编码问题")
        else:
            print(f"  无需修改")
    
    print("彻底编码修复完成！")

if __name__ == "__main__":
    os.chdir('/home/engine/project')
    comprehensive_fix()
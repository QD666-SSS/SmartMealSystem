#!/usr/bin/env python3
import os
import re
import glob

def fix_chinese_strings():
    """手动修复所有中文字符串的u8前缀问题"""
    
    cpp_files = glob.glob('src/*.cpp')
    
    for file_path in cpp_files:
        print(f"处理文件: {file_path}")
        
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # 手动修复特定的中文字符串模式
        # 跳过include语句，只处理cout语句中的中文字符串
        lines = content.split('\n')
        fixed_lines = []
        
        for line in lines:
            # 跳过include语句
            if line.strip().startswith('#include'):
                fixed_lines.append(line)
                continue
            
            # 处理包含中文的字符串字面量
            # 匹配形如 std::cout << "中文" << std::endl; 的模式
            pattern = r'(std::cout << )"([^"]*[一-龯][^"]*)"'
            
            def replace_chinese(match):
                prefix = match.group(1)
                chinese_text = match.group(2)
                return f'{prefix}u8"{chinese_text}"'
            
            # 进行替换
            line = re.sub(pattern, replace_chinese, line)
            
            # 处理std::cout << u8"已有前缀" << std::endl; 这种情况，不需要修改
            fixed_lines.append(line)
        
        new_content = '\n'.join(fixed_lines)
        
        # 保存修改后的文件
        if new_content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(new_content)
            print(f"  已修复编码问题")
        else:
            print(f"  无需修改")
    
    print("编码修复完成！")

if __name__ == "__main__":
    os.chdir('/home/engine/project')
    fix_chinese_strings()
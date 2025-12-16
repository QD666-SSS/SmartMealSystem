#!/usr/bin/env python3
import os
import re
import glob

def fix_encoding_issues():
    """修复所有cpp文件中的UTF-8编码问题"""
    
    cpp_files = glob.glob('src/*.cpp')
    
    for file_path in cpp_files:
        print(f"处理文件: {file_path}")
        
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # 修复缺少u8前缀的中文字符串
        # 匹配形如 "字符串" 但不以u8开头的模式
        pattern = r'(?<!u8)"([^"]*[一-龯][^"]*)"'
        
        def replace_match(match):
            prefix = match.group(1)
            # 如果已经包含u8前缀就不修改
            if 'u8"' in match.string[:match.start()]:
                return match.group(0)
            return f'u8"{prefix}"'
        
        # 进行替换
        content = re.sub(pattern, replace_match, content)
        
        # 保存修改后的文件
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  已修复编码问题")
        else:
            print(f"  无需修改")
    
    print("编码修复完成！")

if __name__ == "__main__":
    os.chdir('/home/engine/project')
    fix_encoding_issues()
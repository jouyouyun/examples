#!/usr/bin/env python3

## 使用aider+gemini-2.5-flash，提示词如下
## 请根据llm/public_domain_code_scanner.md中的描述，实现其功能，生成的代码保存到llm目录下

import os
import re
import csv
import sys

# 不同语言的注释模式定义
# "single_line_prefix": 单行注释前缀列表
# "multi_line_delimiters": 多行注释的起始和结束分隔符元组列表
COMMENT_PATTERNS = {
    ".c": {
        "single_line_prefix": ["//", "#"],
        "multi_line_delimiters": [("/*", "*/")],
    },
    ".cpp": {
        "single_line_prefix": ["//", "#"],
        "multi_line_delimiters": [("/*", "*/")],
    },
    ".h": {
        "single_line_prefix": ["//", "#"],
        "multi_line_delimiters": [("/*", "*/")],
    },
    ".hpp": {
        "single_line_prefix": ["//", "#"],
        "multi_line_delimiters": [("/*", "*/")],
    },
    ".java": {
        "single_line_prefix": ["//"],
        "multi_line_delimiters": [("/*", "*/")],
    },
    ".py": {
        "single_line_prefix": ["#"],
        "multi_line_delimiters": [('"""', '"""'), ("'''", "'''")],
    },
    ".go": {
        "single_line_prefix": ["//"],
        "multi_line_delimiters": [("/*", "*/")],
    },
    ".rs": {
        "single_line_prefix": ["//"],
        "multi_line_delimiters": [("/*", "*/")],
    },
    ".sh": {
        "single_line_prefix": ["#"],
    },
    ".pl": {
        "single_line_prefix": ["#"],
        "multi_line_delimiters": [("=pod", "=cut")], # Perl POD注释
    },
    ".rb": {
        "single_line_prefix": ["#"],
        "multi_line_delimiters": [("=begin", "=end")], # Ruby块注释
    },
    ".js": {
        "single_line_prefix": ["//"],
        "multi_line_delimiters": [("/*", "*/")],
    },
}

# 查找HTTP/HTTPS URL的正则表达式
URL_PATTERN = re.compile(r"https?://\S+")

def clean_line_from_comments(line, lang_patterns, in_multiline_comment_state):
    """
    根据语言注释模式和当前多行注释状态，清除行中的注释。
    返回清除注释后的行以及更新后的多行注释状态。
    """
    cleaned_line = line

    # 1. 处理多行注释
    # 遍历所有可能的多行注释类型
    for start_delim, end_delim in lang_patterns.get("multi_line_delimiters", []):
        if in_multiline_comment_state:
            # 当前处于多行注释块中
            if end_delim in cleaned_line:
                # 注释块在此行结束：取结束分隔符之后的部分
                end_idx = cleaned_line.find(end_delim)
                cleaned_line = cleaned_line[end_idx + len(end_delim):]
                in_multiline_comment_state = False
            else:
                # 注释块继续，整行都被注释
                return "", True # 返回空行并保持多行注释状态
        else:
            # 当前不在多行注释块中
            start_idx = cleaned_line.find(start_delim)
            if start_idx != -1:
                end_idx = cleaned_line.find(end_delim, start_idx + len(start_delim))
                if end_idx != -1:
                    # 注释块在此行开始并结束：移除该块
                    cleaned_line = cleaned_line[:start_idx] + cleaned_line[end_idx + len(end_delim):]
                else:
                    # 注释块在此行开始并延续到下一行：从开始分隔符处移除到行尾
                    cleaned_line = cleaned_line[:start_idx]
                    in_multiline_comment_state = True
        # 如果当前行包含多行注释的起始或结束符，我们认为已处理了该类型
        # 防止同一行内有多种类型的多行注释，通常情况一个文件只用一种。
        # 使用原始行来检查，因为 cleaned_line 可能已被修改
        if start_delim in line or end_delim in line:
            break

    # 2. 处理剩余行中的单行注释
    for prefix in lang_patterns.get("single_line_prefix", []):
        if prefix == "//":
            # 特殊处理 // 前缀，避免误删字符串中的 URL
            # 查找不在引号内的第一个 //
            comment_idx = -1
            # 简单状态机：0=不在引号内, 1=在单引号内, 2=在双引号内
            quote_state = 0
            i = 0
            while i < len(cleaned_line) - 1:
                char = cleaned_line[i]

                # 更新引号状态
                if char == '"' and quote_state == 0:
                    quote_state = 2
                elif char == '"' and quote_state == 2:
                    quote_state = 0
                elif char == "'" and quote_state == 0:
                    quote_state = 1
                elif char == "'" and quote_state == 1:
                    quote_state = 0
                # 简化处理，不考虑转义引号，因为这会使逻辑变得复杂

                # 检查是否为 // 注释，且不在引号内
                if quote_state == 0 and char == '/' and cleaned_line[i+1] == '/':
                    comment_idx = i
                    break # 找到第一个不在引号内的 //

                i += 1
            
            if comment_idx != -1:
                cleaned_line = cleaned_line[:comment_idx]
                break # 找到并处理后，跳出循环
        else: # 对于其他单行注释前缀 (如 #)
            comment_idx = cleaned_line.find(prefix)
            if comment_idx != -1:
                # 移除从注释前缀到行尾的所有内容
                cleaned_line = cleaned_line[:comment_idx]
                break # 找到一个单行注释前缀并处理后，跳出循环

    return cleaned_line, in_multiline_comment_state

def scan_code_path(base_path):
    """
    扫描给定路径下的代码文件，查找公网域名，并过滤掉注释。
    返回一个元组列表：(文件路径, 行号, 原始代码行)
    """
    results = []

    for root, _, files in os.walk(base_path):
        for filename in files:
            filepath = os.path.join(root, filename)
            extension = os.path.splitext(filename)[1].lower() # 统一小写以便查找

            lang_patterns = COMMENT_PATTERNS.get(extension)
            if not lang_patterns:
                continue # 跳过不支持的文件类型

            try:
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    lines = f.readlines()

                in_multiline_comment = False # 用于跟踪当前文件中的多行注释状态
                for i, original_line in enumerate(lines):
                    line_num = i + 1
                    # 移除行尾的换行符，保留其他可能的空白符以便上下文匹配
                    stripped_original_line = original_line.rstrip('\n')

                    # 清除行中的注释并更新多行注释状态
                    cleaned_line, new_in_multiline_comment_state = clean_line_from_comments(
                        stripped_original_line, lang_patterns, in_multiline_comment
                    )
                    in_multiline_comment = new_in_multiline_comment_state # 更新状态以供下一行使用

                    # 如果清除注释后，该行变为空或只包含空白符，则表示该行为注释行，跳过
                    if not cleaned_line.strip():
                        continue

                    # 在清除注释后的行中搜索URL
                    if URL_PATTERN.search(cleaned_line):
                        # 找到URL，添加原始行到结果中
                        results.append((os.path.abspath(filepath), line_num, stripped_original_line))

            except Exception as e:
                # 打印错误信息但不中断扫描其他文件
                print(f"处理文件 {filepath} 时出错: {e}", file=sys.stderr)
    return results

def main():
    if len(sys.argv) != 2:
        print("用法: python public_domain_code_scanner.py <待扫描代码的路径>", file=sys.stderr)
        sys.exit(1)

    scan_path = sys.argv[1]
    if not os.path.exists(scan_path):
        print(f"错误: 路径 '{scan_path}' 不存在。", file=sys.stderr)
        sys.exit(1)

    scanner_results = scan_code_path(scan_path)

    writer = csv.writer(sys.stdout)
    # CSV头部，按照请求格式
    writer.writerow(["代码文件", "行数", "代码行"])
    for result in scanner_results:
        writer.writerow(result)

if __name__ == "__main__":
    main()

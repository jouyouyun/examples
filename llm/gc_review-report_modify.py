import os
import sys
from openpyxl import load_workbook

# 常量定义
SHEET_NAME = '评审信息'
FIELD_NAME = '评审文件归档地址'
FILE_KEYWORD = '评审报告'
EXCEL_EXTENSION = '.xlsx' # 假定为现代 Excel 文件格式

def modify_review_report_file(file_path):
    """
    修改指定的 Excel 文件，将“评审文件归档地址”字段的值置空。
    如果文件被成功修改并保存，则返回 True，否则返回 False。
    """
    try:
        workbook = load_workbook(filename=file_path)
    except Exception:
        # 不打印错误信息，除非明确要求
        return False

    if SHEET_NAME not in workbook.sheetnames:
        return False

    sheet = workbook[SHEET_NAME]
    found_field = False
    
    # 遍历所有单元格以查找 FIELD_NAME
    for row in sheet.iter_rows():
        for cell in row:
            if cell.value == FIELD_NAME:
                # 需要清除的值在 FIELD_NAME 单元格的右侧相邻单元格中
                # openpyxl 的行和列索引是从 1 开始的
                target_cell = sheet.cell(row=cell.row, column=cell.column + 1)
                target_cell.value = None # 将值置空
                found_field = True
                break # 找到字段，无需继续遍历当前行
        if found_field:
            break # 找到字段，无需继续遍历其他行

    if found_field:
        try:
            workbook.save(filename=file_path)
            print(f"成功修改文件: {file_path}") # 只打印被修改的文件
            return True
        except Exception:
            # 不打印错误信息，除非明确要求
            return False
    
    return False # 未找到字段或存在其他问题


def main():
    if len(sys.argv) < 2:
        print("用法: python gc_review-report-modify.py <目录路径>")
        sys.exit(1)

    target_directory = sys.argv[1]

    if not os.path.isdir(target_directory):
        print(f"错误: 指定的路径不是一个有效的目录: {target_directory}")
        sys.exit(1)

    for root, _, files in os.walk(target_directory):
        for filename in files:
            # 检查文件名是否包含关键词并且是 Excel 文件
            if FILE_KEYWORD in filename and filename.endswith(EXCEL_EXTENSION):
                file_path = os.path.join(root, filename)
                modify_review_report_file(file_path)

if __name__ == "__main__":
    main()

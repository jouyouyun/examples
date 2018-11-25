#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from openpyxl import Workbook
from openpyxl.styles import Border, Side, Font, Alignment

border = Border(left=Side(border_style="thin"),
                right=Side(border_style="thin"),
                top=Side(border_style="thin"),
                bottom=Side(border_style="thin"))
titleFont = Font(size=11,
                 color='000000')
textFont = Font(size=10,
                 color='000000')
signAlignment = Alignment(horizontal='general',
                          vertical='bottom',
                          wrap_text=False,
                          indent=0)
centerAlignment = Alignment(horizontal='center',
                            vertical='center',
                            wrap_text=True)
leftAlignment = Alignment(horizontal='general',
                          vertical='center',
                          wrap_text=True)

def style_range(ws, cell_range, border=Border(), fill=None, font=None, alignment=None):
    """
    Apply styles to a range of cells as if they were a single cell.

    :param ws:  Excel worksheet instance
    :param range: An excel range to style (e.g. A1:F20)
    :param border: An openpyxl Border
    :param fill: An openpyxl PatternFill or GradientFill
    :param font: An openpyxl Font object
    """

    top = Border(top=border.top)
    left = Border(left=border.left)
    right = Border(right=border.right)
    bottom = Border(bottom=border.bottom)

    first_cell = ws[cell_range.split(":")[0]]
    if alignment:
        # ws.merge_cells(cell_range)
        first_cell.alignment = alignment

    rows = ws[cell_range]
    if font:
        first_cell.font = font

    for cell in rows[0]:
        cell.border = cell.border + top
    for cell in rows[-1]:
        cell.border = cell.border + bottom

    for row in rows:
        l = row[0]
        r = row[-1]
        l.border = l.border + left
        r.border = r.border + right
        if fill:
            for c in row:
                c.fill = fill

def setSignature(ws, text):
    ws.merge_cells('A1:E1')
    ws['A1'] = text
    # style_range(ws, 'A1:E1', border=border, font=titleFont, alignment=signAlignment)
    return

def setTitle(ws):
    ws.merge_cells('A2:B3')
    ws['A2'] = "软件名称"
    ws.merge_cells('C2:E2')
    ws['C2'] = "软件类型"
    ws['C3'] = "商业软件"
    ws['D3'] = "OEM"
    ws['E3'] = "免费软件"
    # style_range(ws, 'A2:E3', border=border, font=titleFont, alignment=centerAlignment)

if __name__ == "__main__":
    wb = Workbook()
    ws = wb.active
    setSignature(ws, "签名人：________    日期：____年__月__日")
    setTitle(ws)
    ws['A4'] = 'WPS Office'
    ws['C4'] = 'X'
    ws['A5'] = 'Deepin 15.5'
    ws['D5'] = 'X'
    # style_range(ws, 'A4:E7', border=border, font=textFont, alignment=centerAlignment)
    style_range(ws, 'A1:E1', font=titleFont, alignment=signAlignment)
    style_range(ws, 'A2:B3', border=border, font=titleFont, alignment=centerAlignment)
    style_range(ws, 'C2:E2', border=border, font=titleFont, alignment=centerAlignment)
    style_range(ws, 'C3:E3', border=border, font=titleFont, alignment=centerAlignment)
    style_range(ws, 'A4:E4', border=border)
    style_range(ws, 'A5:E5', border=border)
    wb.save("/tmp/test.xlsx")

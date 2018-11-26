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

COL_NAME_LIST = ["A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q"]

def parseRange(range):
    """
    Parse range to get minCol, maxCol, minRow, maxRow
    """
    if range.find(":") == -1:
        return range[0], range[0], int(range[1]), int(range[1])

    list = range.split(":")
    if len(list) != 2:
        return None, None, None, None
    return list[0][0], list[1][0], int(list[0][1]), int(list[1][1])

def setStyleForRange(ws, range, border=Border(), font=None, alignment=None):
    minCol, maxCol, minRow, maxRow = parseRange(range)
    if minCol == None:
        return
    start = COL_NAME_LIST.index(minCol)
    end = COL_NAME_LIST.index(maxCol) + 1
    print("[RANGE] Range: %s(%d, %d)" % (range, start, end))
    cols = COL_NAME_LIST[start:end]
    print("[RANGE] Range col names:", cols)
    for name in cols:
        row = minRow
        print("\t[RANGE]Will set cell: %s" % (name+str(row)))
        while row <= maxRow:
            cell = ws[name+str(row)]
            cell.border = border
            if font != None:
                cell.font = font
            if alignment != None:
                cell.alignment = alignment
            row += 1

def setSignature(ws, text):
    ws.merge_cells('A1:E1')
    ws['A1'] = text
    return

def setTitle(ws):
    ws.merge_cells('A2:B3')
    ws['A2'] = "软件名称"
    ws.merge_cells('C2:E2')
    ws['C2'] = "软件类型"
    ws['C3'] = "商业软件"
    ws['D3'] = "OEM"
    ws['E3'] = "免费软件"

def setRowLine(ws, colStart, start, height, width, name, type):
    idx = COL_NAME_LIST.index(colStart)
    colEnd = COL_NAME_LIST[idx+width-1]
    firstCell = colStart+str(start)
    range = firstCell+":"+(colEnd+str((start+height-1)))
    print("[ROW] Set row range: %s" % range)
    ws.merge_cells(range)
    ws[firstCell] = name
    if type == "business":
        ws["C"+str(start)] = "X"
    elif type == "OEM":
        ws["D"+str(start)] = "X"
    else:
        ws["E"+str(start)] = "X"

if __name__ == "__main__":
    wb = Workbook()
    ws = wb.active
    setSignature(ws, "签名人：________    日期：____年__月__日")
    setTitle(ws)
    setRowLine(ws, "A", 4, 1, 2, "WPS Office", "business")
    setRowLine(ws,"A", 5, 1, 2, "Deepin 15.5", "OEM")
    setRowLine(ws,"A", 6, 1, 2, "Mirosoft Office 2010 Professional", "free")
    rd = ws.row_dimensions[6]
    rd.height = 35
    cd = ws.column_dimensions['A']
    cd.width = 5
    cd = ws.column_dimensions['B']
    cd.width = 5
    setStyleForRange(ws, "A1:E1", font=titleFont, alignment=signAlignment)
    setStyleForRange(ws, "A2:E2", border=border, font=titleFont, alignment=centerAlignment)
    setStyleForRange(ws, "A3:E6", border=border, font=textFont, alignment=centerAlignment)

    wb.save("/tmp/test.xlsx")

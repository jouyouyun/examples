#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from openpyxl import Workbook
from openpyxl.styles import Border, Side, Font, Alignment

__border = Border(left=Side(border_style="thin"),
                right=Side(border_style="thin"),
                top=Side(border_style="thin"),
                bottom=Side(border_style="thin"))
__fontTitle = Font(size=15)
__fontCommon = Font(size=10)
__fontSoftAuth = Font(size=10,
                  color='04ab5f')
__fontSoft = Font(size=10,
                  color='ff6d4d')
__alignSign = Alignment(horizontal='general',
                          vertical='bottom',
                          wrap_text=False,
                          indent=0)
__alignCenter = Alignment(horizontal='center',
                            vertical='center',
                            wrap_text=True)
__alignLeft = Alignment(horizontal='general',
                          vertical='center',
                          wrap_text=True)

__colLabelList = ["A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q"]
__signText = "单位名称（盖章）：________   填表人：______   联系电话：___________   填表日期：____ 年  __ 月 __ 日 "

def parseRange(rangeStr):
    """
    Parse rangeStr to get minCol, maxCol, minRow, maxRow
    """
    if rangeStr.find(":") == -1:
        return rangeStr[0], rangeStr[0], int(rangeStr[1]), int(rangeStr[1])

    list = rangeStr.split(":")
    if len(list) != 2:
        return None, None, None, None
    return list[0][0], list[1][0], int(list[0][1:]), int(list[1][1:])

def setStyleForRange(ws, rangeStr, border=Border(), font=None, alignment=None):
    minCol, maxCol, minRow, maxRow = parseRange(rangeStr)
    if minCol == None:
        return
    start = __colLabelList.index(minCol)
    end = __colLabelList.index(maxCol) + 1
    # print("[RANGE] Range: %s(%d, %d), row: %d ~ %d" % (rangeStr, start, end, minRow, maxRow))
    cols = __colLabelList[start:end]
    # print("[RANGE] Range col names:", cols)
    for name in cols:
        row = minRow
        while row <= maxRow:
            # print("\t[RANGE]Will set cell: %s" % (name+str(row)))
            cell = ws[name+str(row)]
            cell.border = border
            if font != None:
                cell.font = font
            if alignment != None:
                cell.alignment = alignment
            row += 1
    return

def fillTitle(ws,text):
    ws.merge_cells('A2:N2')
    ws['A2'] = text
    ws['A2'].font = __fontTitle
    ws['A2'].alignment = __alignCenter
    return

def fillSign(ws, text):
    ws.merge_cells('A3:N3')
    ws['A3'] = text
    ws['A3'].font = __fontCommon
    ws['A3'].alignment = __alignSign
    return

def fillRowTitle(ws):
    tmpList = ['序号', '部门', '姓名', '计算机编号', '计算机品牌', '软件编号',
                 '软件名称', '软件版本', '许可期限']
    for idx, value in enumerate(tmpList):
        label = __colLabelList[idx]
        ws.merge_cells(label+'4:'+label+'5')
        ws[label+'4'] = value

    ws.merge_cells('J4:L4')
    ws['J4'] = '软件类型'
    ws['J5'] = '商业软件'
    ws['K5'] = 'OEM软件'
    ws['L5'] = '免费软件'

    tmpList = ['安装日期', '可疑']
    for idx, value in enumerate(tmpList):
        label = __colLabelList[idx+12]
        ws.merge_cells(label+'4:'+label+'5')
        ws[label+'4'] = value

    # set column width
    tmpList = [5.5, 10, 10, 6.5, 6.5, 7.5, 22, 10, 10, 5.5, 5.5, 5.5, 10, 5.5]
    for idx, value in enumerate(tmpList):
        col = ws.column_dimensions[__colLabelList[idx]]
        col.width = value

    setStyleForRange(ws, 'A4:N5', border=__border, font=__fontCommon,
                     alignment=__alignCenter)
    return

def fillSoft(ws, soft, rowStr):
    # G ~ M
    auth = 1
    namePrefix = '  '
    if soft['authorized'] == 0:
        auth = 0
        namePrefix = '* '
        ws['G'+rowStr].font = __fontSoft
    else:
        ws['G'+rowStr].font = __fontSoftAuth
    ws['G'+rowStr] = namePrefix + soft['name']
    ws['H'+rowStr] = soft['version']
    ws['I'+rowStr] = soft['licensing_period']
    cr = soft['copyright']
    crValue = '✓'
    crLabel = ''
    if cr == '0':
        crLabel = 'K'
    elif cr == '1':
        crLabel = 'J'
    elif cr == '2':
        crLabel = 'L'
    if len(crLabel) != 0:
        ws[crLabel+rowStr] = crValue
    # TODO: time format: 2018/11/28
    timestamp = soft['installation_time'] # ms
    ws['M'+rowStr] = timestamp
    return auth

def fillRowLine(ws, data, wsStart, seqStart):
    softList = data['softwares']
    softLen = len(softList)
    i = 0
    while i < softLen:
        ws['A'+str(wsStart+i)] = seqStart
        i += 1
        seqStart += 1

    wsEnd = wsStart + softLen - 1
    tmpList = [data['department'], data['owner'], '', data['computer_brand']]
    for idx, value in enumerate(tmpList):
        label = __colLabelList[idx+1]
        ws.merge_cells(label+str(wsStart)+':'+label+str(wsEnd))
        ws[label+str(wsStart)] = value

    auth = 1
    for idx, soft in enumerate(softList):
        idStr = str(wsStart+idx)
        if fillSoft(ws, soft, idStr) == 0:
            auth = 0

    tmpStr = '否'
    if auth == 0:
        tmpStr = '是'
    ws.merge_cells('N'+str(wsStart)+':N'+str(wsEnd))
    ws['N'+str(wsStart)] = tmpStr

    # styles
    setStyleForRange(ws, 'A6:F'+str(wsEnd), border=__border, font=__fontCommon,
                     alignment=__alignCenter)
    setStyleForRange(ws, 'G6:G'+str(wsEnd), border=__border, alignment=__alignCenter)
    setStyleForRange(ws, 'H6:H'+str(wsEnd), border=__border, font=__fontCommon,
                     alignment=__alignCenter)
    setStyleForRange(ws, 'I6:I'+str(wsEnd), border=__border, font=__fontCommon,
                     alignment=__alignLeft)
    setStyleForRange(ws, 'J6:L'+str(wsEnd), border=__border, font=__fontCommon,
                     alignment=__alignCenter)
    setStyleForRange(ws, 'M6:M'+str(wsEnd), border=__border, font=__fontCommon,
                     alignment=__alignLeft)
    setStyleForRange(ws, 'N6:N'+str(wsEnd), border=__border, font=__fontCommon,
                     alignment=__alignCenter)
    return wsEnd + 1, seqStart

def saveFile(wb, outFile):
    try:
        wb.save(outFile)
    except Exception as e:
        print("Failed to save:", e)
        return -1
    return 0

def genFile(infos, outFile):
    wb = Workbook()
    ws = wb.active
    fillTitle(ws, "软件使用情况明细表")
    fillSign(ws, __signText)
    fillRowTitle(ws)
    datas = infos["data"]
    if len(datas) == 0:
        print("No data in infos")
        return saveFile(wb, outFile)

    wsStart = 6
    seqStart = 1
    print("[Gen] start:", len(datas), wsStart, seqStart)
    for idx, data in enumerate(datas):
        print("\t[Data] handle index:", idx, data['owner'], wsStart, seqStart)
        wsStart, seqStart = fillRowLine(ws, data, wsStart, seqStart)

    # set styles
    print("The last ws: %d, seq: %d" % (wsStart, seqStart))
    # endStr = str(wsStart-1)
    # setStyleForRange(ws, 'A4:N5', border=__border, font=__fontCommon,
                     # alignment=__alignCenter)
    # setStyleForRange(ws, 'A6:F'+endStr, border=__border, font=__fontCommon,
                     # alignment=__alignCenter)
    # setStyleForRange(ws, 'G6:G'+endStr, border=__border, alignment=__alignCenter)
    # setStyleForRange(ws, 'H6:H'+endStr, border=__border, font=__fontCommon,
                     # alignment=__alignCenter)
    # setStyleForRange(ws, 'I6:I'+endStr, border=__border, font=__fontCommon,
                     # alignment=__alignLeft)
    # setStyleForRange(ws, 'J6:L'+endStr, border=__border, font=__fontCommon,
                     # alignment=__alignCenter)
    # setStyleForRange(ws, 'M6:M'+endStr, border=__border, font=__fontCommon,
                     # alignment=__alignLeft)
    # setStyleForRange(ws, 'N6:N'+endStr, border=__border, font=__fontCommon,
                     # alignment=__alignCenter)
    return saveFile(wb, outFile)

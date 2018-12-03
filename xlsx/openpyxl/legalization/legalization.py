#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import time
from openpyxl import Workbook
from openpyxl.styles import Border, Side, Font, Alignment

class Legalization(object):
    wb = Workbook()
    ws = wb.active
    border_thin = Border(left=Side(border_style='thin'),
            right=Side(border_style='thin'),
            top=Side(border_style='thin'),
            bottom=Side(border_style='thin'))
    font_title = Font(size=12)
    font_common = Font(size=10)
    font_soft = Font(size=10,
            color='ff6d4d')
    font_soft_auth = Font(size=10,
            color='04ab5f')
    alignment_sign = Alignment(horizontal='general',
            vertical='center',
            wrap_text=False,
            indent=0)
    alignment_center = Alignment(horizontal='center',
            vertical='center',
            wrap_text=True)
    alignment_left = Alignment(horizontal='general',
            vertical='center',
            wrap_text=True)
    column_name_labels = ['A','B','C','D','E','F','G','H','I','J','K','L',
            'M','N','O','P','Q']

    def __init__(self):
        return

    def fill_title(self, text):
        self.ws.merge_cells('A2:N2')
        self.ws['A2'] = text
        self.ws['A2'].font = self.font_title
        self.ws['A2'].alignment = self.alignment_center
        self.ws.row_dimensions[2].height = 40
        return

    def fill_sign(self, data=None):
        self.ws.merge_cells('A3:N3')
        self.ws['A3'] = self.get_sign(data)
        self.ws['A3'].font = self.font_common
        self.ws['A3'].alignment = self.alignment_sign
        self.ws.row_dimensions[3].height = 30
        return

    def generate_file(self, out_file):
        """ Implement in child class """
        return

    def get_sign(self, data=None):
        if data == None:
            return '单位名称（盖章）：_________   填表人：_______   联系电话：___________    填表日期：____ 年  __ 月 __ 日'

        s = '单位名称（盖章）：_________   填表人：'
        applicant = data['applicant']
        if len(applicant) == 0:
            applicant = '_________'
        s += applicant + '   联系电话：'
        phone = data['telephone']
        if len(phone) == 0:
            phone = '_____________'
        s += phone + '    填表日期：____ 年  __ 月 __ 日'
        return s

    def set_range_style(self, cell_range, border=Border(), font=None, align=None):
        """
        Set the cells style, contains: border, font and alignment

        @ws: the workbook sheet handler
        @cell_range: the cells range, format: <cell_start>:<cell_end>
        @border: the cell border
        @font: the cell font
        @align: the cell alignment
        """

        min_col, max_col, min_row, max_row = self._parse_cell_range(cell_range)
        if min_col == None:
            return
        start = self.column_name_labels.index(min_col)
        end = self.column_name_labels.index(max_col) + 1
        for label in self.column_name_labels[start:end]:
            row = min_row
            while row <= max_row:
                cell = self.ws[label+str(row)]
                cell.border = border
                if font != None:
                    cell.font = font
                if align != None:
                    cell.alignment = align
                row += 1
        return

    def save_file(self, filename):
        """
        Save the xlsx file

        @wb: the workbook handler
        @filename: the output xlsx file name
        """

        # mkdir parent dir
        dir_path = os.path.dirname(filename)
        if len(dir_path) != 0:
            try:
                os.makedirs(dir_path, 0o755, True)
            except Exception as e:
                print('Failed to make dir:', filename, e)
                return

        # save file
        try:
            self.wb.save(filename)
        except Exception as e:
            print('Failed to save file:', filename, e)
        return

    def format_timestamp(self, stamp):
        """
        Format ms timestamp

        @stamp: ms timestamp, such as: 1539157392000
        """
        l = len(stamp)
        if l != 13:
            return ''
        t = time.localtime(int(stamp[:l-3]))
        return time.strftime('%Y/%m/%d', t)

    def _parse_cell_range(self, cell_range):
        """
        Parse cell range, return min column, max column, min row number, max row number

        @cell_range: format <cell>:<cell>
        """

        if cell_range.find(':') == -1:
            return cell_range[0], cell_range[0], int(cell_range[1:]), int(cell_range[1:])
        strv = cell_range.split(':')
        if len(strv) != 2 or len(strv[1]) == 0:
            return None, None, None, None
        return strv[0][0], strv[1][0], int(strv[0][1:]), int(strv[1][1:])

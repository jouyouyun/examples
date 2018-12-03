#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from legalization import Legalization

class Overview(Legalization):
    def __init__(self, infos):
        self.__infos = infos
        return

    def __fill_row_title(self):
        # title 1
        self.ws.merge_cells('A4:B4')
        self.ws['A4'] = '人员情况'
        self.ws['C4'] = '总人数'
        self.ws.merge_cells('D4:F4')
        self.ws.merge_cells('G4:H4')
        self.ws['G4'] = '使用计算机人数'
        self.ws.merge_cells('I4:M4')

        # title 2
        self.ws.merge_cells('A5:B5')
        self.ws['A5'] = '计算机情况'
        self.ws['C5'] = '服务器数'
        self.ws.merge_cells('D5:F5')
        self.ws.merge_cells('G5:H5')
        self.ws['G5'] = '台式机数'
        self.ws.merge_cells('I5:J5')
        self.ws['K5'] = '便携机数'
        self.ws.merge_cells('L5:M5')

        # title 3
        strv = ['序号', '软件编号', '软件名称', '软件版本', '软件类型', '采购时间', 
                '采购金额(单位:元)', '许可类型', '许可数量', '可使用版本', 
                '许可期限', '序列号', '安装数量']
        for idx, value in enumerate(strv):
            label = self.column_name_labels[idx]
            self.ws[label+'6'] = value

        self.set_range_style('A4:M6', border=self.border_thin, 
                font=self.font_common, align=self.alignment_center)

        # column width
        widths = [5.5, 5.5]
        for idx, value in enumerate(widths):
            label = self.column_name_labels[idx]
            self.ws.column_dimensions[label].width = value
        return

    def __file_row_line(self, data, row_start, seq_start):
        return

    def generate_file(self, out_file):
        self.fill_title('软件使用情况汇总表')
        self.fill_sign(self.__infos['title'])
        self.__fill_row_title()
        return self.save_file(out_file)

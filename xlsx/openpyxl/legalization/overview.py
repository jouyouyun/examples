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
        widths = [5, 5, 12]
        for idx, value in enumerate(widths):
            label = self.column_name_labels[idx]
            self.ws.column_dimensions[label].width = value
        self.ws.column_dimensions['J'].width = 10
        self.ws.column_dimensions['K'].width = 12
        self.ws.column_dimensions['L'].width = 10
        return

    def __fill_bussiness_line(self, data, row_start, seq_start):
        l1 = 0
        for ver in data['Versions']:
            l1 += len(ver['Values'])
        tmp_list = [seq_start, '', data['Name'], '', '商业软件', '', '', '', '']
        row_num = row_start
        sstr = str(row_num)
        estr = str(row_num+l1-1)
        for idx, value in enumerate(tmp_list):
            label = self.column_name_labels[idx]
            self.ws.merge_cells(label+sstr+':'+label+estr)
            self.ws[label+sstr] = value
        for ver in data['Versions']:
            values = ver['Values']
            v_len = len(values)
            sstr = str(row_num)
            estr = str(row_num+v_len-1)
            self.ws.merge_cells('J'+sstr+':J'+estr)
            self.ws['J'+sstr] = ver['Version']
            for value in values:
                sstr = str(row_num)
                self.ws['K'+sstr] = value['LicensingPeriod']
                self.ws['M'+sstr] = value['Number']
                row_num += 1
        self.set_range_style('A'+str(row_start)+':E'+str(row_num-1),
                border=self.border_thin, font=self.font_common,
                align=self.alignment_center)
        self.set_range_style('F'+str(row_start)+':F'+str(row_num-1),
                border=self.border_thin, font=self.font_common,
                align=self.alignment_left)
        self.set_range_style('G'+str(row_start)+':J'+str(row_num-1),
                border=self.border_thin, font=self.font_common,
                align=self.alignment_center)
        self.set_range_style('K'+str(row_start)+':L'+str(row_num-1),
                border=self.border_thin, font=self.font_common,
                align=self.alignment_left)
        self.set_range_style('M'+str(row_start)+':M'+str(row_num-1),
                border=self.border_thin, font=self.font_common,
                align=self.alignment_center)
        return row_num, seq_start+1

    def __fill_other_line(self, data, row_start, seq_start, oem):
        sstr = str(row_start)
        self.ws['A'+sstr] = seq_start
        seq_start += 1
        self.ws['C'+sstr] = data['Name']
        if oem == False:
            self.ws['D'+sstr] = '--'
            self.ws['E'+sstr] = '免费软件'
        else:
            # self.ws['D'+sstr] = data['']
            self.ws['E'+sstr] = 'OEM软件'
        self.ws.merge_cells('F'+sstr+':L'+sstr)
        self.ws['F'+sstr] = '--'
        num = 0
        for ver in data['Versions']:
            for value in ver['Values']:
                num += value['Number']
        self.ws['M'+sstr] = num
        self.set_range_style('A'+sstr+':M'+sstr, border=self.border_thin,
                font=self.font_common, align=self.alignment_center)
        return row_start+1, seq_start

    def generate_file(self, out_file):
        self.fill_title('软件使用情况汇总表')
        self.fill_sign(self.__infos['title'])
        self.__fill_row_title()
        overview_list = self.__infos['overview']

        row_start = 7
        seq_start = 1
        # bussiness
        for data in overview_list[0]:
            row_start, seq_start = self.__fill_bussiness_line(data, row_start, seq_start)

        # oem
        for data in overview_list[1]:
            row_start, seq_start = self.__fill_other_line(data,
                    row_start, seq_start, True)

        # free
        for data in overview_list[2]:
            row_start, seq_start = self.__fill_other_line(data,
                    row_start, seq_start, False)

        return self.save_file(out_file)

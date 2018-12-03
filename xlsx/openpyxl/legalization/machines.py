#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from legalization import Legalization

class Machines(Legalization):
    def __init__(self, infos):
        self.__infos = infos
        return

    def __fill_row_title(self):
        strv = ['序号', '部门', '姓名', '计算机编号', '计算机品牌', '软件编号',
                '软件名称', '软件版本', '许可期限']
        for idx, value in enumerate(strv):
            label = self.column_name_labels[idx]
            self.ws.merge_cells(label+'4:'+label+'5')
            self.ws[label+'4'] = value

        self.ws.merge_cells('J4:L4')
        self.ws['J4'] = '软件类型'
        self.ws['J5'] = '商业软件'
        self.ws['K5'] = 'OEM软件'
        self.ws['L5'] = '免费软件'

        strv = ['安装日期', '可疑']
        for idx, value in enumerate(strv):
            label = self.column_name_labels[idx+12]
            self.ws.merge_cells(label+'4:'+label+'5')
            self.ws[label+'4'] = value

        # set column width
        widths = [5.5, 10, 10, 6.5, 6.5, 7.5, 22, 10, 10, 5.5, 5.5, 5.5, 10, 5.5]
        for idx, value in enumerate(widths):
            col = self.ws.column_dimensions[self.column_name_labels[idx]]
            col.width = value
        self.set_range_style('A4:N5', border=self.border_thin, 
                font=self.font_common, align=self.alignment_center)
        return

    def __fill_row_line(self, data, row_start, seq_start):
        soft_list = data['softwares']
        soft_len = len(soft_list)
        i = 0
        while i < soft_len:
            self.ws['A'+str(row_start+i)] = seq_start
            seq_start += 1
            i += 1

        row_end = row_start + soft_len -1
        start_str = str(row_start)
        end_str = str(row_end)
        strv = [data['department'], data['owner'], '', data['computer_brand']]
        for idx, value in enumerate(strv):
            label = self.column_name_labels[idx+1]
            self.ws.merge_cells(label+start_str+':'+label+end_str)
            self.ws[label+start_str] = value

        auth = True
        for idx, soft in enumerate(soft_list):
            row_num = str(row_start+idx)
            if self.__fill_software(soft, row_num) == False:
                auth = False

        tmp = '否'
        if auth == False:
            tmp = '是'
        self.ws.merge_cells('N'+start_str+':N'+end_str)
        self.ws['N'+start_str] = tmp

        # set styles
        end_str = str(row_end)
        self.set_range_style('A6:F'+end_str, border=self.border_thin, 
                font=self.font_common, align=self.alignment_center)
        self.set_range_style('G6:G'+end_str, border=self.border_thin, 
                align=self.alignment_center)
        self.set_range_style('H6:H'+end_str, border=self.border_thin, 
                font=self.font_common, align=self.alignment_center)
        self.set_range_style('I6:I'+end_str, border=self.border_thin, 
                font=self.font_common, align=self.alignment_left)
        self.set_range_style('J6:L'+end_str, border=self.border_thin, 
                font=self.font_common, align=self.alignment_center)
        self.set_range_style('M6:M'+end_str, border=self.border_thin, 
                font=self.font_common, align=self.alignment_left)
        self.set_range_style('N6:N'+end_str, border=self.border_thin, 
                font=self.font_common, align=self.alignment_center)
        return row_end + 1, seq_start

    def __fill_software(self, soft, row_str):
        # G ~ M
        auth = True
        name_prefix = '  '
        if soft['authorized'] == 0:
            auth = False
            name_prefix = '* '
            self.ws['G'+row_str].font = self.font_soft
        else:
            self.ws['G'+row_str].font = self.font_soft_auth

        self.ws['G'+row_str] = name_prefix+soft['name']
        self.ws['H'+row_str] = soft['version']
        self.ws['I'+row_str] = soft['licensing_period']

        cr = soft['copyright']
        cr_text = '✓'
        cr_label = ''
        if cr == '0':
            cr_label = 'K'
        elif cr == '1':
            cr_label = 'J'
        elif cr == '2':
            cr_label = 'L'
        if len(cr_label) != 0:
            self.ws[cr_label+row_str] = cr_text

        self.ws['M'+row_str] = self.format_timestamp(soft['installation_time'])
        return auth

    def generate_file(self, out_file):
        self.fill_title('软件使用情况明细表')
        self.fill_sign(self.__infos['title'])
        self.__fill_row_title()
        datas = self.__infos['machines']
        data_len = len(datas)
        if data_len == 0:
            return self.save_file(out_file)
        row_start = 6
        seq_start = 1
        for data in datas:
            row_start, seq_start = self.__fill_row_line(data, row_start, seq_start)
        return self.save_file(out_file)

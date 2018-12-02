#!/usr/bin/env python3
# -*- coding: utf-8 -*-

class Human(object):
    age = 0
    name = ''

    def __init__(self, sex):
        self.sex = sex
    def get_name(self):
        return self.name
    def get_age(self):
        return self.age
    def __del__(self):
        pass

class Man(Human):
    def __init__(self):
        self.sex = True

def main():
    Man.school = '' # 给类绑定变量
    m = Man()
    m.school = 'USTC'
    m.name = 'Joy'
    m.age = 19
    """
    动态类型的语言在创建实例后, 可以给实例绑定任何的属性和方法, 但这些绑定只对当前实例有效
    如果要对所以实例生效, 可以在创建实例前给动态的给类绑定
    """
    m.parent = 'Big Joy' # 实例变量, 只对当前实例有效
    print(m.get_name(), m.get_age(), m.sex, m.parent, m.school)

if __name__ == '__main__':
    main()

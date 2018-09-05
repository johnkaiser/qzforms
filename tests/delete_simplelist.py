#!/usr/bin/env python
from selenium import webdriver
from selenium.webdriver.support.ui import Select
import selenium
from time import sleep
from qztest import * 

def remove_blue_css(f):
    print "remove_blue_css"
    f.find_element_by_xpath('//input[@value="css files"]').click()
    #sleep(1)
    list_table_edit(f, 'filename', 'blue.css')
    #sleep(1)
    try:
        f.find_element_by_xpath('//input[@value="Delete"]').click()
    except selenium.common.exceptions.NoSuchElementException:
        return False
    #sleep(1)
    return True

print "delete_simplelist"
f = login("test","42",'http://deb.donefor.net/qz/login')
delete_menu_item(f, "main", "simplelist")
edit_form(f, 'simplelist')

new_page_click(f, 'page_menus')
empty_grid(f)

new_page_click(f, 'page_css')
empty_grid(f)

new_page_click(f, 'page_js')
empty_grid(f)

new_page_click(f, 'Prompt_Rules') 
empty_onetable(f)

new_page_click(f, 'Table_Action')
empty_onetable(f)

print "removing simplelist"
edit_form(f, 'simplelist')
new_page_click(f, 'Delete')

remove_blue_css(f)    

new_page_click(f, 'Main_Menu')
new_page_click(f, 'Logout')
sleep(5)
f.close()
print "Fin"

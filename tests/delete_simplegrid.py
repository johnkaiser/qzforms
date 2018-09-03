#!/usr/bin/env python
from selenium import webdriver
from selenium.webdriver.support.ui import Select
import selenium
from time import sleep
from qztest import * 




print "create_simplegrid.py ver 0.0001"
f = login("test","420",'http://deb.donefor.net/qz/login')
delete_menu_item(f, 'main', 'simplegrid')
edit_form(f, 'simplegrid')

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

print "removing simplegrid"
edit_form(f, 'simplegrid')
new_page_click(f, 'Delete')
new_page_click(f, 'Logout')
sleep(5)
f.quit()

print "Fin"

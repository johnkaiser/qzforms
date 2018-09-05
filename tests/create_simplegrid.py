#!/usr/bin/env python
from selenium import webdriver
from selenium.webdriver.support.ui import Select
import selenium
from time import sleep
from qztest import * 

def create_form(f):
    print "create_form"
    new_page_click(f, "Form Development")
    new_page_click(f, "Forms")
    
    f.find_element_by_id('form_name').send_keys('simplegrid')
    handler_name = Select(f.find_element_by_id('new_handler_name'))
    handler_name.select_by_visible_text('grid')
    new_page_click(f, 'Insert')
    
    f.find_element_by_id('schema_name').send_keys('public')
    f.find_element_by_id('table_name').send_keys('simplegrid')
    f.find_element_by_id('pkey_btn').click()
    f.find_element_by_id('pkey[0]').send_keys('n')
    pc = Select(f.find_element_by_id('prompt_container'))
    pc.select_by_visible_text('fieldset')
    new_page_click(f, 'Submit')

    return

def table_action_edit(f):
    print "table_action_edit"
    new_page_click(f, 'Table_Action')

    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text('edit')
    new_page_click(f, 'Insert')

    helpful = f.find_element_by_xpath('//textarea[@name="helpful_text"]')
    helpful.send_keys('Edit simplegrid table.')
    sql = f.find_element_by_xpath('//textarea[@name="sql"]')
    sql.send_keys("""SELECT  n,"data" FROM simplegrid ORDER BY n """)
    new_page_click(f, 'Submit')

    return

def table_action_save(f):
    print "table_action_save"
    new_page_click(f, 'Table_Action')

    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text('save')
    new_page_click(f, 'Insert')

    helpful = f.find_element_by_xpath('//textarea[@name="helpful_text"]')
    helpful.send_keys('Save a simplegrid table.')
    sql = f.find_element_by_xpath('//textarea[@name="sql"]')
    sql.send_keys("""SELECT  1""")
    new_page_click(f, 'Submit')

    return


def table_action_insert_row(f):
    print "table_action_insert_row"
    new_page_click(f, 'Table_Action')

    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text('insert_row')
    new_page_click(f, 'Insert')

    sql = f.find_element_by_xpath('//textarea[@name="sql"]')
    sql.send_keys("""INSERT INTO simplegrid ("data") VALUES ($1)""")

    f.find_element_by_id('fieldnames_btn').click()
    f.find_element_by_id('fieldnames[0]').send_keys('data')

    new_page_click(f, 'Submit')

    return

def table_action_update_row(f):
    print "table_action_update_row"
    new_page_click(f, 'Table_Action')

    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text('update_row')
    new_page_click(f, 'Insert')

    sql = f.find_element_by_xpath('//textarea[@name="sql"]')
    sql.send_keys("""UPDATE simplegrid SET "data" = $2 WHERE n = $1 """)

    f.find_element_by_id('fieldnames_btn').click()
    f.find_element_by_id('fieldnames[0]').send_keys('n')
    f.find_element_by_id('fieldnames_btn').click()
    f.find_element_by_id('fieldnames[1]').send_keys('data')

    new_page_click(f, 'Submit')

    return

def table_action_delete_row(f):
    print "table_action_delete_row"
    new_page_click(f, 'Table_Action')

    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text('delete_row')
    new_page_click(f, 'Insert')

    sql = f.find_element_by_xpath('//textarea[@name="sql"]')
    sql.send_keys("""DELETE FROM simplegrid WHERE n = $1""")

    f.find_element_by_id('fieldnames_btn').click()
    f.find_element_by_id('fieldnames[0]').send_keys('n')

    new_page_click(f, 'Submit')

    return


def add_prompt_rule(f):
    print "add_prompt_rule"
    new_page_click(f, 'Prompt_Rules')

    f.find_element_by_id('fieldname').send_keys('n')
    new_page_click(f, 'Insert')

    prompt_type =  Select(f.find_element_by_id('prompt_type'))
    prompt_type.select_by_visible_text('input_hidden')
    f.find_element_by_id('size').send_keys('6')
    f.find_element_by_id('maxlength').send_keys('6')
    new_page_click(f, 'Submit')

    f.find_element_by_id('fieldname').send_keys('data')
    new_page_click(f, 'Insert')

    prompt_type =  Select(f.find_element_by_id('prompt_type'))
    prompt_type.select_by_visible_text('textarea')
    f.find_element_by_id('el_class').send_keys('blue')
    f.find_element_by_id('rows').send_keys('5')
    f.find_element_by_id('cols').send_keys('40')
    new_page_click(f, 'Submit')

    return

def add_page_js(f):
    print "add_page_js"
    new_page_click(f, 'page_js')

    f.find_element_by_id('page_js_add_row').click()
    sleep(0.1)
    f.find_element_by_id('sequence[0]').send_keys('1')
    action =  Select(f.find_element_by_id('filename[0]'))
    action.select_by_visible_text('qzforms.js')
    new_page_click(f, 'Save')

    return

def add_page_css(f):
    print "add_page_css"
    new_page_click(f, 'page_css')

    f.find_element_by_id('page_css_add_row').click()
    sleep(0.1)

    f.find_element_by_id('sequence[0]').send_keys('1')
    action =  Select(f.find_element_by_id('filename[0]'))
    action.select_by_visible_text('qzforms.css')
    f.find_element_by_id('page_css_add_row').click()
    new_page_click(f, 'Save')

    return

def add_page_menus(f):
    print "add_page_menus"
    new_page_click(f, 'page_menus')

    f.find_element_by_id('page_menus_add_row').click()
    sleep(0.1)
    menu_names =  Select(f.find_element_by_id('menu_name[0]'))
    f.find_element_by_id('action[0]').send_keys('any')
    menu_names.select_by_visible_text('main')
    new_page_click(f, 'Save')

    return

print "create_simplegrid.py ver 0.0001"
f = login("test","42",'http://deb.donefor.net/qz/login')
create_form(f)
edit_form(f, 'simplegrid')
table_action_edit(f)
table_action_save(f)
table_action_insert_row(f)
table_action_update_row(f)
table_action_delete_row(f)
add_prompt_rule(f)
add_to_menu(f, 'main', '110', 'simplegrid', 'edit')

edit_form(f, 'simplegrid')
add_page_js(f)
add_page_css(f)
add_page_menus(f)

new_page_click(f, 'Logout')
sleep(5)
f.quit()
print "Fin"

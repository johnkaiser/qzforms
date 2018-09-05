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
    
    f.find_element_by_id('form_name').send_keys('simplelist')
    handler_name = Select(f.find_element_by_id('new_handler_name'))
    handler_name.select_by_visible_text('onetable')
    new_page_click(f, 'Insert')
    
    f.find_element_by_id('schema_name').send_keys('public')
    f.find_element_by_id('table_name').send_keys('simplelist')
    f.find_element_by_id('pkey_btn').click()
    f.find_element_by_id('pkey[0]').send_keys('n')
    pc = Select(f.find_element_by_id('prompt_container'))
    pc.select_by_visible_text('fieldset')
    new_page_click(f, 'Submit')

    return

def table_action_create(f):
    print "table_action_create"
    new_page_click(f, 'Table_Action')

    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text('create')
    new_page_click(f, 'Insert')

    helpful = f.find_element_by_xpath('//textarea[@name="helpful_text"]')
    helpful.send_keys('Create a new simplelist row.')
    sql = f.find_element_by_xpath('//textarea[@name="sql"]')
    sql.send_keys("""SELECT ''::text "data" """)
    new_page_click(f, 'Submit')

    return

def table_action_list(f):
    print "table_action_list"
    new_page_click(f, 'Table_Action')

    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text('list')
    new_page_click(f, 'Insert')

    helpful = f.find_element_by_xpath('//textarea[@name="helpful_text"]')
    helpful.send_keys('List simplelist')

    sql = f.find_element_by_xpath('//textarea[@name="sql"]')
    sql.send_keys( 'SELECT n,"data" from simplelist ORDER BY n');
    new_page_click(f, 'Submit')

    return

def table_action_insert(f):
    print "table_action_insert"
    new_page_click(f, 'Table_Action')

    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text('insert')
    new_page_click(f, 'Insert')

    helpful = f.find_element_by_xpath('//textarea[@name="helpful_text"]')
    helpful.send_keys('Insert new row.')

    sql = f.find_element_by_xpath('//textarea[@name="sql"]')
    sql.send_keys("""INSERT INTO simplelist ("data") VALUES ($1) """)

    f.find_element_by_id('fieldnames_btn').click()
    f.find_element_by_id('fieldnames[0]').send_keys('data')
    new_page_click(f, 'Submit')

    return

def table_action_edit(f):
    print "table_action_edit"
    new_page_click(f, 'Table_Action')

    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text('edit')
    new_page_click(f, 'Insert')

    helpful = f.find_element_by_xpath('//textarea[@name="helpful_text"]')
    helpful.send_keys('Edit a row.')

    sql = f.find_element_by_xpath('//textarea[@name="sql"]')
    sql.send_keys(""" SELECT n,"data" from simplelist WHERE n = $1 """)

    f.find_element_by_id('fieldnames_btn').click()
    f.find_element_by_id('fieldnames[0]').send_keys('n')

    new_page_click(f, 'Submit')

    return

def table_action_update(f):
    print "table_action_update"
    new_page_click(f, 'Table_Action')

    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text('update')
    new_page_click(f, 'Insert')

    helpful = f.find_element_by_xpath('//textarea[@name="helpful_text"]')
    helpful.send_keys('Update a row.')

    sql = f.find_element_by_xpath('//textarea[@name="sql"]')
    sql.send_keys("""
        UPDATE simplelist
        SET "data" = $2
        WHERE n = $1
         """)

    f.find_element_by_id('fieldnames_btn').click()
    f.find_element_by_id('fieldnames[0]').send_keys('n')
    f.find_element_by_id('fieldnames_btn').click()
    f.find_element_by_id('fieldnames[1]').send_keys('data')
    new_page_click(f, 'Submit')

    return

def table_action_delete(f):
    print "table_action_delete"
    new_page_click(f, 'Table_Action')

    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text('delete')
    new_page_click(f, 'Insert')
    
    helpful = f.find_element_by_xpath('//textarea[@name="helpful_text"]')
    helpful.send_keys('delete a row.')

    sql = f.find_element_by_xpath('//textarea[@name="sql"]')
    sql.send_keys(""" DELETE FROM simplelist WHERE n = $1 """)

    f.find_element_by_id('fieldnames_btn').click()
    f.find_element_by_id('fieldnames[0]').send_keys('n')
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

def create_blue_css_file(f):
    print"add_css_file"
    new_page_click(f, 'css files')
    fn = f.find_element_by_id('filename')
    fn.send_keys('blue.css')
    new_page_click(f, 'Insert')

    data = f.find_element_by_id('data')
    data.send_keys('textarea.blue {background: lightblue;}')
    new_page_click(f, 'Submit')

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
    sleep(0.1)

    f.find_element_by_id('sequence[1]').send_keys('2')
    action =  Select(f.find_element_by_id('filename[1]'))
    action.select_by_visible_text('blue.css')
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

def add_prompt_rule(f):
    print "add_prompt_rule"
    new_page_click(f, 'Prompt_Rules')

    f.find_element_by_id('fieldname').send_keys('data')
    new_page_click(f, 'Insert')

    prompt_type =  Select(f.find_element_by_id('prompt_type'))
    prompt_type.select_by_visible_text('textarea')
    f.find_element_by_id('el_class').send_keys('blue')
    #  XXXXX Wrong regex dialect, fix syntax for a valid test.
    #f.find_element_by_id('regex_pattern').send_keys('[A-Za-z0-9 ./+]*')
    f.find_element_by_id('rows').send_keys('5')
    f.find_element_by_id('cols').send_keys('40')
    new_page_click(f, 'Submit')

    return

def add_inline_css(f):
    print "add_inline_css"
    new_page_click(f, 'inline_css')

    table = f.find_element_by_id('list')
    trs = table.find_elements_by_tag_name('tr')
    foundit = False

    for tr in trs:
        menu_names = tr.find_elements_by_class_name('action')
        if len(menu_names) < 1: continue
        print "-- ", menu_names[0].text
        if menu_names[0].text == 'edit':
            foundit = True
            form = tr.find_elements_by_tag_name('form')[0]
            form.submit()
            sleep(1)
            break
    if (foundit==False):
        print "form not found"
        return
    inline_css = f.find_element_by_id('inline_css')
    inline_css.send_keys('input.n {background: lightgreen}')
    new_page_click(f, 'Submit')


print "create_simplelist.py ver 0.0006"
f = login("test","42",'http://deb.donefor.net/qz/login')
create_form(f)
edit_form(f,'simplelist')
table_action_create(f)
table_action_insert(f)
table_action_edit(f)
table_action_update(f)
table_action_delete(f)
table_action_list(f)
add_to_menu(f, 'main', '100', 'simplelist', 'list')
create_blue_css_file(f)
edit_form(f,'simplelist')
add_page_js(f)
add_page_css(f)
add_page_menus(f)
add_prompt_rule(f)
edit_form(f,'simplelist')
add_inline_css(f)
new_page_click(f, 'Logout')
sleep(5)
f.close()
print "fin"


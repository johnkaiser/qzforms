from selenium import webdriver
from selenium.webdriver.support.ui import Select
from time import sleep
import selenium

def login(user, passwd, url):
    print "login"
    f = webdriver.Firefox()
    f.get(url)
    
    u=f.find_element_by_id("user")
    u.click()
    u.clear()
    u.send_keys(user)
    
    p=f.find_element_by_id("password")
    p.clear()
    p.send_keys(passwd)
    
    new_page_click(f, 'Login')

    return f

def list_table_edit(f, col_class, btn_text):
    #
    # A onetable list will produce an html table
    # where each html row corresponds to one Postgres table row.
    # Find the given btn_text in the column col_class
    # and press the Edit button for that row.
    #
    print "list_table_edit", col_class, btn_text
    table = f.find_element_by_id('list')
    trs = table.find_elements_by_tag_name('tr')
    found_it = False
    for tr in trs:
        tr_cols = tr.find_elements_by_class_name(col_class)
        if len(tr_cols) < 1: continue
        if tr_cols[0].text == btn_text:
            found_it = True
            form = tr.find_elements_by_tag_name('form')[0]
            form.submit()
            break
    if found_it == False: return False

    nap_time = 0.2
    max_naps = 25
    found_it = False
    naps = 0
    ## Wait for the current page to be invalid. 
    while not found_it and naps < max_naps:
        try:
            form.find_elements_by_id('something_not_there')
            print "waiting for", col_class, btn_text, max_naps - naps
            naps += 1
            sleep(nap_time)
        except selenium.common.exceptions.StaleElementReferenceException:
            # Success!
            return True

    return found_it

def edit_form(f, form_name):
    print "edit_form", form_name
    new_page_click(f, 'Form Development')
    new_page_click(f, 'Forms')
    list_table_edit(f, 'form_name', form_name)

    return

## Reference:
## http://www.obeythetestinggoat.com/how-to-get-selenium-to-wait-for-page-load-after-a-click.html

def new_page_click(f, btn_text):
    nap_time = 0.2
    max_naps = 25
    found_it = False
    naps = 0
    #p='//input[@value="%s"] | //button[@type="submit]"/contains(text(),"%s")' 
    pinput = '//input[@value="%s"]' % (btn_text,)
    pbtn = '//button[text()="%s"]' % (btn_text,)
    
    while not found_it and naps < max_naps:
        try:
            old_btn =  f.find_element_by_xpath(pinput)
            found_it = True
            old_btn.click()
        except selenium.common.exceptions.NoSuchElementException:
            None
        try:
            if not found_it:
                old_btn =  f.find_element_by_xpath(pbtn)
                found_it = True
                old_btn.click()
        except selenium.common.exceptions.NoSuchElementException:
            None
        if not found_it: print "button '%s' not found, will retry" % (btn_text,)
        naps += 1
        sleep(nap_time)
    if not found_it: raise selenium.common.exceptions.NoSuchElementException
    while naps < max_naps:
        try:
            old_btn.find_elements_by_id('something_not_there')
            print "waiting for", btn_text,max_naps - naps
            naps += 1
            sleep(nap_time)
        except selenium.common.exceptions.StaleElementReferenceException:
            # Success!
            return True


def delete_menu_item(f, menu_name, item_name):
    print "delete_menu_item", menu_name, item_name
    new_page_click(f, "Form Development")
    new_page_click(f, "Menu Menu")
    new_page_click(f, "All Menus")
    list_table_edit(f, 'menu_name', menu_name)    
    new_page_click(f, "Menu Items")
    list_table_edit(f, 'target_form_name', item_name)
    try:
        f.find_element_by_xpath('//input[@value="Delete"]').click()
    except selenium.common.exceptions.NoSuchElementException:
        return False
    return True



def empty_grid(f):
    table = f.find_element_by_id('grid_edit_table')
    btns = table.find_elements_by_tag_name('button')
    for btn in btns:
        if btn.text == 'delete': btn.click()
    f.find_element_by_xpath('//input[@value="Save"]').click()

    
def remove_one_row(f):
    print "remove_one_row"
    table = f.find_element_by_id('list')
    trs = table.find_elements_by_tag_name('tr')
    last = len(trs) - 1
    tr = trs[last]
    try:
        tr.find_element_by_xpath('//input[@value="Edit"]').click()
        #sleep(1)
        f.find_element_by_xpath('//input[@value="Delete"]').click()
        return True
    except selenium.common.exceptions.NoSuchElementException:
        return False


def empty_onetable(f):
    print "empty_onetable"
    while True:
        if not remove_one_row(f): break


def add_to_menu(f, menu_name, sequence, item_name, action_name):
    print "add_to_menu", menu_name, item_name, action_name
    new_page_click(f, 'Form Development')
    new_page_click(f, 'Menu Menu')
    new_page_click(f, 'All Menus')

    table = f.find_element_by_id('list')
    trs = table.find_elements_by_tag_name('tr')
    foundit = False
    for tr in trs:
        menu_names = tr.find_elements_by_class_name('menu_name')
        if len(menu_names) < 1: continue
        print "-- ", menu_names[0].text
        if menu_names[0].text == menu_name:
            foundit = True
            form = tr.find_elements_by_tag_name('form')[0]
            form.submit()
            break
    if (foundit==False):
        print "form not found"
        return

    new_page_click(f, "Menu Items")

    f.find_element_by_id('menu_item_sequence').send_keys(sequence)
    new_page_click(f, "Insert")

    f.find_element_by_id('target_form_name').send_keys(item_name)
    action = Select(f.find_element_by_id('action'))
    action.select_by_visible_text(action_name)
    f.find_element_by_id('menu_text').send_keys(item_name)
    new_page_click(f, 'Submit')

    return



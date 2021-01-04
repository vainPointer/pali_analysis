#!/usr/bin/env python
#-*- coding: UTF-8 -*-
import os
import re
import sqlite3

db_pm   = sqlite3.connect("pm.db")
db_comp = sqlite3.connect("comp.db")
db_to   = sqlite3.connect("dict_compound.db")

parts_of_word = {}

def init_sqlite(db):
    create_dict  = '''CREATE TABLE dict (
        pali    TEXT    NOT NULL,
        parts   TEXT    NOT NULL);'''
    create_index = "CREATE UNIQUE INDEX dict_index ON dict (pali);"
    try:
        db.execute(create_dict)
        db.execute(create_index)
        db.commit()
    except:
        print("[-] database already existed!")

def filter_from(db):
    global parts_of_word
    results = db.execute("SELECT pali,parts FROM dict;")
    for row in results:
        word  = row[0].encode('utf-8')
        parts = row[1].encode('utf-8')
        if '-' in parts:
            continue
        parts = re.sub("\(.*\)", '', parts).strip()
        parts = re.sub("¿", '', parts).strip()
        if parts:
            if word not in parts_of_word:
                parts_of_word[word] = parts

def insert_into(db):
    db_c = db.cursor()
    insert_word="INSERT INTO dict VALUES (\"{}\", \"{}\");"
    for word in parts_of_word:
        db.execute(insert_word.format(word, parts_of_word[word]))
    db.commit()

init_sqlite(db_to)
db_to.execute("DELETE FROM dict;")
db_to.commit()

filter_from(db_pm)
filter_from(db_comp)
insert_into(db_to)

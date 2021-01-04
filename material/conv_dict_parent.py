#!/usr/bin/env python
#-*- coding: UTF-8 -*-
# 
# sqlite3 sys_regular.db "select pali from dict where pali!=parent" \
# | sort | uniq -c | wc -l gives the unique "pali!=parent" words (278543)
# SELECT pali,parent FROM dict WHERE pali IN (SELECT pali FROM dict 
# GROUP BY pali HAVING count(*)>=2 ORDER BY pali); gives the dup-words
import sqlite3

db_reg   = sqlite3.connect("sys_regular.db")
db_irreg = sqlite3.connect("sys_irregular.db")
db_to    = sqlite3.connect("dict_parent.db")

parent_of_word = {}

def init_sqlite(db):
    create_dict  = '''CREATE TABLE dict (
        pali    TEXT    NOT NULL,
        parent  TEXT    NOT NULL);'''
    create_index = "CREATE UNIQUE INDEX dict_index ON dict (pali);"
    try:
        db.execute(create_dict)
        db.execute(create_index)
        db.commit()
    except:
        print("[-] database already existed!")

def filter_from(db):
    global parent_of_word
    results = db.execute("SELECT pali,parent FROM dict;")
    for row in results:
        word  = row[0].encode('utf-8')
        parent = row[1].encode('utf-8')
        if word == parent:
            continue
        # utf-8 encoding have problem with ā etc.
        len_parent = len(row[1])
        if not len(parent):
            continue
        if len_parent > (len(row[0]) + 2):
            continue
        if word not in parent_of_word:
            parent_of_word[word]     = [parent, len_parent]
        elif len_parent > parent_of_word[word][1]:
            parent_of_word[word]     = [parent, len_parent]

def insert_into(db):
    db_c = db.cursor()
    insert_word="INSERT INTO dict VALUES (\"{}\", \"{}\");"
    for word in parent_of_word:
        db.execute(insert_word.format(word, parent_of_word[word][0]))
    db.commit()

init_sqlite(db_to)
db_to.execute("DELETE FROM dict;")
db_to.commit()

filter_from(db_reg)
filter_from(db_irreg)
insert_into(db_to)

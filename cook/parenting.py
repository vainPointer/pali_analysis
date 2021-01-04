#!/usr/bin/env python
#-*- coding: UTF-8 -*-
import re
import sys
import json
import struct
import sqlite3

parent_index     = {}
parent_index_num = 0

def __append_to_parent_index(parent):
    global parent_index
    global parent_index_num
    parent_index[parent] = parent_index_num
    parent_index_num    += 1
    if parent_index_num % 2000 == 0:
        print("current_index_num: {}".format(parent_index_num))

''' Get parent of a word. If the word cannot find in dict_parent, return the 
original word. No matter the word is found or not, insert the word into the
parent_index database as the word index for the simplified version of text.
'''
def _parent_of(word):
    global parent_cursor
    parent = word
    results = list(parent_cursor.execute(
        "SELECT parent FROM dict where pali=\"" + word + "\";"
        ))
    if len(results):
        parent = results[0][0].encode('utf-8')
    if parent not in parent_index:
        #print("{} append word: {}".format(parent_index_num, parent))
        if parent == '':
            print(word)
        __append_to_parent_index(parent)
    # return the index of the word in parent_index
    return str(parent_index[parent])

''' Split the compund in ['word1', 'word2', ...] form if found else return 0
'''
def _split_compound(comp):
    global comp_cursor
    results = list(comp_cursor.execute(
        "SELECT parts FROM dict where pali=\"" + comp + "\";"
        ))
    if len(results):
        results = results[0][0].encode('utf-8').split('+')
        split = []
        # filter parts in results
        for part in results:
            part = part.strip()
            part = re.sub("\(.*\)", '', part).strip()
            part = re.sub("\[.*\]", '', part).strip()
            part = re.sub("-", '', part).strip()
            if len(part) > 1:
                split.append(part)
        # end of filter parts
        if len(split):
            return split
    return 0

''' First try to split the word as a compound. If so, parent the splited words
respectively and return the result in 'word1 word2' form.
'''
def simplify(word):
    split = _split_compound(word)
    if split:
        split = list(map(_parent_of, split))
        return ' '.join(split)
    else:
        return _parent_of(word)

''' Given a line, convert all words to the parent forms and compounds splited.
'''
def convert(line):
    # remove punctuations and digits, where .[]{}+- should have \ escape from
    # regular expression, and – (UTF8: E28093) is different from -.
    excluded_char = "–‘’',\.\[\]\{\}\+\-;0123456789"
    line = re.sub("[{}]+".format(excluded_char), '', line).strip()

    # split line into words list
    words = line.split(' ')
    while '' in words:
        words.remove('')

    # simplify words list to the parent form and compunds splited
    words = list(map(simplify, words))
    words = ' '.join(words)

    if words:
        words = [struct.pack("H", int(word)) for word in words.split(' ')]
    return words

if __name__ == "__main__":
    #infile = open("../tmp/sample.in.txt")
    infile = open("../tmp/pali_text.txt")
    parent_dict = sqlite3.connect("../material/dict_parent.db")
    parent_cursor = parent_dict.cursor()
    comp_dict = sqlite3.connect("../material/dict_compound.db")
    comp_cursor = comp_dict.cursor()

    #output = open("../tmp/sample.out.txt", "w")
    output = open("../tmp/pali_text_index.txt", "w")

    for line in infile:
        line = line.strip('\n')
        line = convert(line)
        for word in line:
            output.write(word)
        output.write('\r\n')
        
    infile.close()
    output.close()

    #output = open("../tmp/sample.index.txt", "w")
    output = open("../tmp/pali_index.txt", "w")
    for word in parent_index:
        output.write(word + ' ')
        output.write(str(parent_index[word]) + '\n')
    output.close()

    print("final index number: {}".format(parent_index_num))


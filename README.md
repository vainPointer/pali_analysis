# Pāli Analysis

## What is this repository

This repository is some work related to [pāliwiki](https://github.com/iapt-platform/mint), which is a Tipiṭaka reading, learning and translating platform. 

## Explanation of the folders

### material

includes the materials for cook, they are basicly convert from others.

From [pāliwiki](https://github.com/iapt-platform/mint):

- `pali_text.tgz` is the full text of Tipiṭaka.

- `dict_parent.db` is convert from  `sys_regular.db` and `sys_irregular.db`. 
  - The `sys_regular.db` is a regular inflection dictionary infered by all possible grammer rules. The parent of a word is not the stem nor the root. For example, "gam->gaccha(ti)->gacchanta->gacchato", each word's previous is called the parent of the word.
  - The `sys_irregular.db` is the dictionary of irregular word entered manually.

- `dict_compound.db` is convert from `pm.db` and  `comp.db`, it contains the split of each compound word.
  - `pm.db` is the pāli-myanmar dictionary.
  - `comp.db` is the results of splits using algorithm.

### cook

includes preprocessing scripts:

- `parenting.py` convert each word of pali_text.txt to its parent using `dict_parent.db` and `dict_compound.db`.

### sentence

includes codes used for analysis the sentences in Tipiṭaka:

- Get similar sentences using jaccard similarity.


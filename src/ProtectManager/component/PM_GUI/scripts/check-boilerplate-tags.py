"""
$file: check_boilerplate_tags.py
$author: Protect Manager

check PLACEHOLDER service tags (e.g. SERVICENAME, IMAGENAME etc)
"""
import os
import re

regex = re.compile(r'\<((IMAGE|SERVICE|USER)_?NAME|AUTHOR(MAIL)?|DESCRIPTION)\>')
fregex = re.compile(r'\.(ya?ml|java|sh)$|^\.?docker.+$', re.IGNORECASE)

exclude_dirs = set(['.git', '__pycache__'])

found = set()
print('searching for service boilerplate tags...')

for root, dirs, files in os.walk('./', topdown=True):
    dirs[:] = [d for d in dirs if d not in exclude_dirs]
    for fname in files:
        if not fregex.search(fname) or re.search(r'^check.*boil', fname):
            continue
        path = '{}/{}'.format(root, fname)
        with open(path) as f:
            nof_line = 0
            for line in f:
                nof_line = nof_line + 1
                result = regex.search(line)
                if result:
                    found.add(path)
                    print('{}:{} {}'.format(path, nof_line, result.group(1)))

print('total boilerplate tags: {}'.format(len(found)))
exit(len(found))

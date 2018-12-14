#! /usr/bin/env python3

import os
import re
import shutil
import string
import sys

TEMPLATE_EXTENSION_ID = "projectid"
TEMPLATE_EXTENSION_NAMESPACE = "ProjectNamespace"
TEMPLATE_EXTENSION_PRETTYNAME = "Template"
TEMPLATE_EXTENSION_DIRNAME = "templateExtension"

RE_ID = "^([a-z0-9]+)$"
RE_NAMESPACE = "^([A-Za-z][A-Za-z0-9]+)$"
RE_PRETTYNAME = "^([A-Za-z0-9 _\\-]+)$"


# Check sanity of input

if len(sys.argv) != 4:
    u = "Usage: create_plugin.py <id [a-z0-9]> <namespace> <Pretty Name>\n"\
        "e.g. create_plugin.py applications applicatoins Applications"
    sys.stderr.write(u)
    sys.exit(1)

new_id, new_namespace, new_prettyname = sys.argv[1:]

if not re.match(RE_ID, new_id):
    e = "ID has to match " + RE_ID + "\n"
    sys.stderr.write(e)
    sys.exit(1)

if not re.match(RE_NAMESPACE, new_namespace):
    e = "Namespace has to match " + RE_NAMESPACE + "\n"
    sys.stderr.write(e)
    sys.exit(1)

if not re.match(RE_PRETTYNAME, new_prettyname):
    e = "Pretty Name has to match " + RE_PRETTYNAME + "\n"
    sys.stderr.write(e)
    sys.exit(1)

if not os.path.isdir(TEMPLATE_EXTENSION_DIRNAME):
    e = "Template extension missing. Are we in the src/plugins directory?"
    sys.stderr.write(e)
    sys.exit(1)


print("Copying template extension.")
shutil.copytree(TEMPLATE_EXTENSION_DIRNAME, new_id)


print("Adjusting file contents…")
os.chdir(new_id)
for root, dirs, files in os.walk("."):
    for name in files:
        relative_file_path = os.path.join(root, name)
        print("\t%s" % relative_file_path)
        with open(relative_file_path) as file:
            content = file.read()
            content = re.sub(TEMPLATE_EXTENSION_ID, new_id, content)
            content = re.sub(TEMPLATE_EXTENSION_NAMESPACE, new_namespace, content)
            content = re.sub(TEMPLATE_EXTENSION_PRETTYNAME, new_prettyname, content)
        with open(relative_file_path, "w") as file:
            file.write(content)
os.chdir("..")


print("Adding 'add_subdirectory' section to the CMakeLists.txt in the root dir.")
with open("CMakeLists.txt", "a") as file:
    cmake_hunk = '\noption(BUILD_{0} "Build the extension" ON)\n'\
                 'if (BUILD_{0})\n'\
                 '    add_subdirectory({1})\n'\
                 'endif()\n'.format(new_id.upper(), new_id)
    file.write(cmake_hunk)

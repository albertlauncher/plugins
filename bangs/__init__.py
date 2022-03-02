# -*- coding: utf-8 -*-

"""This is a simple python template extension.

This extension should show the API in a comprehensible way. Use the module docstring to provide a \
description of the extension. The docstring should have three paragraphs: A brief description in \
the first line, an optional elaborate description of the plugin, and finally the synopsis of the \
extension.

Synopsis: <trigger> [delay|throw] <query>"""

from albert import *
import os
from time import sleep


__title__ = "Bangs"
__version__ = "0.0.1"
__triggers__ = "!"
__authors__ = "Dyefferson Azevedo"
# __exec_deps__ = ["whatever"]

iconPath = os.path.dirname(__file__) + "/bang.svg"


def handleQuery(query):
    if not query.isTriggered:
        return

    results = []

    bangName = query.rawString.split(" ")[0][1:]
    searchQuery = " ".join(query.rawString.split(" ")[1:])

    item = Item()

    item.icon = iconPath

    if bangName and searchQuery:
        item.text = "Bang " + bangName + ' used to search for "' + searchQuery + '".'
    else:
        item.text = "Bang!"
        item.subtext = "Search faster!"

    item.actions = [
        UrlAction(text="UrlAction", url="https://duckduckgo.com/?q=" + query.rawString)
    ]
    results.append(item)

    return results

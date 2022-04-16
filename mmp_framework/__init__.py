"""
Python library with an interface for the MMP framework.
Source Code and Examples: https://github.com/Ciaoc/mmp_framework
"""
import os

import cppyy

# Setting global include-paths for cppyy
try:
    current_dir = os.path.dirname(__file__)
except NameError:
    current_dir = os.getcwd()
cppresource_dir = os.path.join(current_dir, 'cppresources')

cppalg_dir = os.path.join(cppresource_dir, 'algorithm')
cppexpression_dir = os.path.join(cppresource_dir, 'expressiontree')
cppexpressionutil_dir = os.path.join(cppexpression_dir, 'util')

cppyy.add_include_path(cppalg_dir)
cppyy.add_include_path(cppexpression_dir)
cppyy.add_include_path(cppexpressionutil_dir)

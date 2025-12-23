# Re-export the native module for 'import zxingcpp' compatibility.
# Named init.py instead of __init__.py to avoid import conflicts when running from this directory.
from .zxingcpp import *

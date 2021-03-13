import logging
import warnings
import os

class VSWarningsHandler(logging.Handler):
    def __init__(self, message_list):
        logging.Handler.__init__(self)
        self.message_list = message_list
    def emit(self, message_record):
        self.message_list.append(message_record.getMessage())

warning_messages = []

def custom_formatwarning(message, category, filename, lineno, line=None):
    if filename == "" or os.path.samefile(__file__, filename):
        return f'{category.__name__} : {filename} : line {lineno - 31}:  {message}\n'
    else:
        return f'{category.__name__} : {filename} : line {lineno}:  {message}\n'

warnings.formatwarning = custom_formatwarning
warnings.simplefilter("default")

logging.captureWarnings(True)
vs_warnings_handler = VSWarningsHandler(warning_messages)
logging.getLogger('py.warnings').addHandler(vs_warnings_handler)

def get_warnings():
    return warning_messages

from vapoursynth import core
core.std.LoadPlugin(r"{{p}}")

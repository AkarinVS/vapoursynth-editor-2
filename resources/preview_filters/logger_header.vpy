import logging
import warnings

class VSWarningsHandler(logging.Handler):
    def __init__(self, message_list):
        logging.Handler.__init__(self)
        self.message_list = message_list
    def emit(self, message_record):
        self.message_list.append(message_record.getMessage())

warning_messages = []
warnings.formatwarning = lambda message, category, filename, lineno, line=None: f'{category.__name__} : line {lineno-19}:  {message}\n'
warnings.simplefilter("default")

logging.captureWarnings(True)
logging.getLogger('py.warnings').addHandler(VSWarningsHandler(warning_messages))

def get_warnings():
    return warning_messages

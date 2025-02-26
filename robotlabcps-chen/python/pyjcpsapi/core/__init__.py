import os
import sys

core_full_path = os.path.dirname(os.path.abspath(__file__))
if sys.platform == 'win32':
    if sys.version_info.major == 3:
        if sys.version_info.minor == 7:
            sys.path.append(os.path.join(core_full_path, 'py37'))
            from .py37 import pycpsapi
        else:
            sys.path.append(os.path.join(core_full_path, 'py38'))
            from .py38 import pycpsapi
else:
    sys.path.append(os.path.join(core_full_path, 'py37_linux'))
    from .py37_linux import pycpsapi

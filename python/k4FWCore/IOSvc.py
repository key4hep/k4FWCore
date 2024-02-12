from Configurables import IOSvc as IO
import os

class IOSvc:

    def __init__(self, *args, **kwargs):

        self._svc = IO(**kwargs)

    def __getattr__(self, attr):
        return getattr(self._svc, attr)

    def __setattr__(self, attr, value):
        print(f'IOSvc: {attr} {value}')
        if attr == '_svc':
            super().__setattr__(attr, value)
            return

        if attr == 'input':
            if isinstance(value, str):
                value = [value]
            for inp in value:
                if os.path.dirname(inp) and not os.path.exists(inp):
                    os.makedirs(os.path.dirname(inp))
        if attr == 'output':
            if os.path.dirname(value) and not os.path.exists(os.path.dirname(value)):
                os.makedirs(os.path.dirname(value))
        setattr(self._svc, attr, value)

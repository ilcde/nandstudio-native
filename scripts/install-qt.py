"""Pinned aqt 3.3.0 adapter for Qt's split 6.11 Windows repositories.

Upstream issue: https://github.com/miurahr/aqtinstall/issues/1007
Only repository selection changes; aqt still verifies downloaded checksums.
"""
import importlib.metadata
import runpy
from aqt.archives import QtArchives

if importlib.metadata.version('aqtinstall') != '3.3.0':
    raise RuntimeError('This compatibility adapter requires aqtinstall 3.3.0')
original = QtArchives._arch_ext

def arch_extension(self):
    if self.os_name == 'windows' and str(self.version) == '6.11.1' and self.target == 'desktop':
        if self.arch != 'win64_msvc2022_64':
            raise RuntimeError('Only the pinned MSVC 2022 x64 kit is supported by this adapter')
        return '_msvc2022_64'
    return original(self)

QtArchives._arch_ext = arch_extension
if __name__ == '__main__':
    runpy.run_module('aqt', run_name='__main__')

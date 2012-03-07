"""Script to update the src/common/src/version.h with SVN information.
   Requires Python to be installed (and in your path), as well as the commandline
   version of SVN.
"""

import os
import shutil
import subprocess

(SCRIPTS_ROOT, _) = os.path.split(os.path.abspath(__file__))
(SVN_ROOT, _) = os.path.split(SCRIPTS_ROOT)

path_to_common = os.path.join(SVN_ROOT, 'src', 'common', 'src')
path_to_version_file = os.path.join(path_to_common, 'version.h')

f = open(path_to_version_file, 'r')

new_lines = []
svn_rev = subprocess.Popen(["svnversion", ('%s' % SVN_ROOT)], stdout=subprocess.PIPE).communicate()[0]
for line in f.readlines():
    if line.startswith('#define __BUILD__ '):
        new_lines.append('#define __BUILD__ "%s"\n' % (svn_rev.strip('\n').strip('\r')))
        pass
    else:
        new_lines.append(line)
        pass
    pass
    
f.close()

f = open(path_to_version_file, 'w')
f.writelines(new_lines)
f.close()
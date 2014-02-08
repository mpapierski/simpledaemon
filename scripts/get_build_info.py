import uuid
import sys
import subprocess
import datetime
import time
from email import utils

_, output = sys.argv

# This is for include guard
guid = str(uuid.uuid4()).replace('-', '_')
guard = 'BUILD_INFO_{0}'.format(guid)

# Get current date with timezone
try:
  stdout = subprocess.check_output(['/bin/date', '-R'], stderr=subprocess.PIPE)
  build_date = stdout.strip()
except subprocess.CalledProcessError:
  nowdt = datetime.datetime.now()
  nowtuple = nowdt.timetuple()
  nowtimestamp = time.mktime(nowtuple)
  build_date = utils.formatdate(nowtimestamp)

# Get repo info
stdout = subprocess.check_output(['/usr/bin/git', 'rev-parse', 'HEAD'])
commit = stdout.strip()
commit = commit[:8]

with open(output, 'wb') as fout:
  fout.write('''#if !defined({0})
#define {0}
#define BUILD_INFO_DATE "{1}"
#define BUILD_INFO_COMMIT "{2}"
#endif /* {0} */
'''.format(guard, build_date, commit))

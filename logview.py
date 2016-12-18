#!/usr/bin/env python

# Some sample log file lines
## 1398352765.838431 49 get_session_state:71 logged_in-CONNECTION_OK user:qz
## 1398352765.838438 49 do_page:149 do_page begin handler onetable
## 1398352765.838447 49 doc_from_file:13 begin doc_from_file
## 1398352765.838745 49 doc_from_file:41 doc_from_file complete
## 1398352765.838768 49 perform_action:511 perform_action started table_action_getall_22ahp2qoXd7Mt3L
## 1398352765.839950 49 perform_action:521 perform_action completed PGRES_TUPLES_OK 
## 1398352765.839961 49 onetable_getall:360 perform_action  result PGRES_TUPLES_OK returned 6 cols 32 rows
## 1398352765.839975 49 get_pgtype_datum:478 get_pgtype_datum(table_action,lookup_name)

from datetime import datetime 
from operator import itemgetter, attrgetter
import sys

if len(sys.argv) >= 2:
  filename = sys.argv[1]
else:
  filename = "qz.log"

if len(sys.argv) >= 3:
   request_id = int(sys.argv[2])
else:
   request_id = False

logfilelines = open(filename).readlines()
logfile = [ x.strip().split() for x in logfilelines if len(x) > 1 ]

def getkey(line):
    try:
        key = int(line[1])
    except ValueError:
        key = None
    except IndexError:
        key = None
    return key

logfile.sort(key=getkey)
#timeused = "duration"
timeused = "step"
last_seq_nbr = -1
last_timestamp = 0
duration_start = 0


for fields in logfile:
    try:
        timestamp = float(fields[0])
        seq_nbr = int(fields[1])
        if request_id and seq_nbr != request_id:
            continue
        if seq_nbr != last_seq_nbr:
             print "\n%s %05d" % (datetime.fromtimestamp(timestamp).ctime(), seq_nbr)
             last_timestamp = timestamp
             duration_start = timestamp
        
        if timeused == "duration":
            delta = timestamp - duration_start; 
        elif timeused == "step":
            delta = timestamp - last_timestamp
        else:
            delta = timestamp
  
        print "%18.6f %05d %s" % ( delta, seq_nbr, " ".join(fields[2:]))
  
        last_timestamp = timestamp
        last_seq_nbr = seq_nbr
    except IndexError:
        #print "IndexError", line.strip()
        pass
    except ValueError:
        pass




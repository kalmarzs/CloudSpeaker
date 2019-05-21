#!/usr/bin/env python3

import subprocess
import sys

def play(url, station_id):
    cmd=["/usr/bin/mplayer", url]
    file=open('/tmp/cloudspeaker.info', 'w+')
    pid=subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE).pid
    file.write(str(pid)+" "+str(station_id))

def stop():
    subprocess.Popen(["/usr/bin/pkill", "mplayer"])
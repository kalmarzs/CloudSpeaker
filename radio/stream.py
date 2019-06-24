#!/usr/bin/env python3

import subprocess
import sys

def play(url, station_id, device):
    print(device)
    cmd=["/usr/bin/mplayer", device, url]
    print(cmd)
    file=open('/tmp/cloudspeaker.info', 'w+')
    pid=subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE).pid
    file.write(str(pid)+" "+str(station_id))

def stop():
    subprocess.Popen(["/usr/bin/pkill", "mplayer"])
    file=open('/tmp/cloudspeaker.info', 'w')
    file.close()

def now_playing():
    subprocess.Popen(["/usr/bin/ps aux "])
#!/usr/bin/env python3
import subprocess
import sys

def play(track):
    path_w_track = "/media/cloudspeaker_mp3/"+track
    cmd=["/usr/bin/mplayer", path_w_track]
    file=open('/tmp/cloudspeaker.info', 'w+')
    pid=subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE).pid
    file.write(str(pid)+" "+str(path_w_track))

def stop():
    subprocess.Popen(["/usr/bin/pkill", "mplayer"])
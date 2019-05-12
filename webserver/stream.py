#!/usr/bin/env python3

import subprocess

def play(url):
    cmd=["/usr/bin/mplayer", url]
    subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)

def stop():
    subprocess.Popen(["/usr/bin/pkill", "mplayer"])
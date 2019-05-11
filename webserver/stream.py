#!/usr/bin/env python3

import subprocess

def play(url):
    subprocess.Popen(["/usr/bin/mplayer", url])

def stop():
    subprocess.Popen(["/usr/bin/pkill", "mplayer"])
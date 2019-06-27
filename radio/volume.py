#!/usr/bin/env python3

from subprocess import call, check_output

def set(value):
    call(["/usr/bin/amixer", "-c1", "sset", "Master", value+"%"])

def mute():
    call(["/usr/bin/amixer", "set", "PCM", "mute"])

def unmute():
    call(["/usr/bin/amixer", "set", "PCM", "unmute"])

def level():
    output = check_output("/usr/bin/amixer -c1 sget Master | tail -1 | awk '{print $4}' | cut -c 2-4", shell=True)
    #output = output.decode('ascii')
    output = int(output)
    return output
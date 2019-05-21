#!/usr/bin/env python3

from subprocess import call

def volup():
    call(["/usr/bin/amixer", "set", "PCM", "532+"])

def voldn():
    call(["/usr/bin/amixer", "set", "PCM", "532-"])

def mute():
    call(["/usr/bin/amixer", "set", "PCM", "mute"])

def unmute():
    call(["/usr/bin/amixer", "set", "PCM", "unmute"])

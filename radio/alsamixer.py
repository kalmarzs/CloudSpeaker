#!/usr/bin/env python3
import alsaaudio
m = alsaaudio.Mixer(alsaaudio.mixers[0]) 
m.setvolume(90)

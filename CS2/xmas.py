#!/usr/bin/env python3
import RPi.GPIO as GPIO
import sys as sys

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(24, GPIO.OUT)

def on():
    GPIO.output(24, GPIO.HIGH)

def off():
    GPIO.output(24, GPIO.LOW)

if __name__ == '__main__':
    globals()[sys.argv[1]]()

#!/usr/bin/env python3

from flask import Flask, render_template, request

app = Flask(__name__,
    static_url_path='',
    static_folder='static',
    template_folder='templates')

import sqlite3 as sql
import radio as program
import alsaaudio
import os as os
import RPi.GPIO as GPIO

global mixer
mixer = alsaaudio.Mixer('Headphone')

@app.route('/')
def main():
    conn = sql.connect("/opt/CS2/radio.db")
    conn.row_factory = sql.Row
    cur = conn.cursor()
    cur.execute("select id, name, logo from stations order by id;")
    rows = cur.fetchall()
    conn.close()
    tracks = os.listdir('/media/cloudspeaker_mp3')
    vol_level = mixer.getvolume()
    temp = open("/opt/CS2/temp.txt", "r")
    #statinfo = os.stat("/opt/CS2/temp.txt")
    return render_template('index.html',tracks = tracks, rows = rows, vol_level = vol_level[0], temp = temp.read())

@app.route('/setvol/<value>')
def setvol(value):
    mixer.setvolume(int(value))
    return ("Volume adjusted to "+value+"%")

@app.route('/radio/<action>/<id>')
def radio(action, id):
    listen = program.Radio(id)
    if action == "on":
      GPIO.setmode(GPIO.BCM)
      GPIO.setup(24, GPIO.OUT)
      GPIO.output(24, GPIO.HIGH)
      listen.start()
      return ("Radio Started: "+id)

    if action == "off":
      GPIO.setmode(GPIO.BCM)
      GPIO.setup(24, GPIO.OUT)
      GPIO.output(24, GPIO.LOW)
      listen.stop()
      return("Radio off")

    if action == "status":
      res = listen.status()
      return(res)

@app.route('/temp', methods=['GET'])
def temp():
    data = request.args.get('temp')
    file = ('/opt/CS2/temp.txt')
    if data == "query":
      temp = open(file, "r")
      return('A homerseklet: '+temp.read()+' &deg;C')
    else:
      write = open(file, 'w')
      write.write(data)
      write.close
      return(data)

if __name__ == "__main__":
   app.run(host='0.0.0.0', port=80, debug=True)

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

global mixer
mixer = alsaaudio.Mixer('PCM')

@app.route('/')
def main():
    conn = sql.connect("/opt/CloudSpeaker/radio/radio.db")
    conn.row_factory = sql.Row
    cur = conn.cursor()
    cur.execute("select id, name, logo from stations order by id;")
    rows = cur.fetchall()
    conn.close()
    tracks = os.listdir('/media/cloudspeaker_mp3')
    vol_level = mixer.getvolume()
    return render_template('index.html',tracks = tracks, rows = rows, vol_level = vol_level[0])

@app.route('/setvol/<value>')
def setvol(value):
    mixer.setvolume(int(value))
    return ("Volume adjusted to "+value+"%")

@app.route('/radio/<action>/<id>')
def radio(action, id):
    listen = program.Radio(id)
    if action == "on":
      listen.start()
      return ("Radio Started: "+id)

    if action == "off":
      listen.stop()
      return("Radio off")

    if action == "status":
      res = listen.status()
      return(res)

@app.route('/temp', methods=['GET'])
def temp():
    data = request.args.get('temp')
#    data = ("25")
    print(data)
    file = ('/opt/CloudSpeaker/radio/temp.txt')
    write = open(file, 'w')
    write.write(data)
    write.close
    return render_template('temp.html', data = data)

if __name__ == "__main__":
   app.run(host='0.0.0.0', port=80, debug=True)

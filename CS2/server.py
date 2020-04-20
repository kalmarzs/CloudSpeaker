#!/usr/bin/env python3

from flask import Flask, render_template, request
app = Flask(__name__)
import sqlite3 as sql
import stream as stream
import alsaaudio
import media as media
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

@app.route('/turn_off')
def turn_off():
    stream.stop()
    return ("Radio Off")

@app.route('/setvol/<value>')
def setvol(value):
    mixer.setvolume(int(value))
    return ("Volume adjusted.")

@app.route('/play/<track>')
def play(track):
    media.play(track)
    return ("nothing")

@app.route('/station/<station_id>')
def station(station_id):
    print (station_id)
    conn = sql.connect("/opt/CloudSpeaker/radio/radio.db")
    conn.row_factory = sql.Row
    cur = conn.cursor()
    cur.execute("select url, name from stations where id="+station_id+" limit 1;")
    conn.commit()
    rows=cur.fetchall()
    for url in rows:
      stream.play(url[0], station_id)
    return ("Radio Started: "+url[1])

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
   app.run(host='0.0.0.0', port=8080, debug=True)

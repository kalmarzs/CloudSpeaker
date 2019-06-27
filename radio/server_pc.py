#!/usr/bin/env python3

from flask import Flask, render_template, request
app = Flask(__name__)
import sqlite3 as sql
import stream as stream
import volume as volume

@app.route("/")
def main():
   conn = sql.connect("radio.db")
   conn.row_factory = sql.Row
   cur = conn.cursor()
   cur.execute("select id, logo from stations order by id;")
   rows = cur.fetchall();
   return render_template("index.html",rows = rows)

@app.route("/knob")
def knob():
   conn = sql.connect("radio.db")
   conn.row_factory = sql.Row
   cur = conn.cursor()
   cur.execute("select id, logo from stations order by id;")
   rows = cur.fetchall();
   vol = volume.level();
   print(vol)
   return render_template("knob.html",rows = rows, vol = vol)


@app.route('/turn_off')
def turn_off():
    print ("turn off")
    stream.stop()
    return ("nothing")

@app.route('/volume_set/<level>')
def volume_set(level):
    print ("volume set to level: "+level)
    volume.set(level)
    return ("nothing")

@app.route('/mute')
def mute():
    volume.mute()
    print ("mute")
    return ("nothing")

@app.route('/station/<station_id>')
def station(station_id):
    device = "-ao alsa:device=hw=1.0"
    print (station_id)
    conn = sql.connect("radio.db")
    conn.row_factory = sql.Row
    cur = conn.cursor()
    cur.execute("select url from stations where id="+station_id+" limit 1;")
    conn.commit()
    rows=cur.fetchall()
    for url in rows:
        print (url[0])
    stream.play(url[0], station_id, device)
    return ("nothing")

@app.route("/admin")
def admin():
   return render_template('admin.html')

if __name__ == "__main__":
   app.run(host='0.0.0.0', port=80, debug=True)

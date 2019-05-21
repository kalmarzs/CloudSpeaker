#!/usr/bin/env python3

from flask import Flask, render_template, request
app = Flask(__name__)
import sqlite3 as sql
import stream as stream
import volume as volume

@app.route("/")
def main():
    conn = sql.connect("/home/bond/CloudSpeaker/webserver/radio.db")
    conn.row_factory = sql.Row
    cur = conn.cursor()
    cur.execute("select id, logo from stations order by id;")
    rows = cur.fetchall()
    conn.close()
#    if (rows["pid"] !=""):
#     style='style="filter: grayscale(0%);"'
#    else:
#     style='style="filter: grayscale(100%);"'
    return render_template("index.html",rows = rows)

@app.route('/turn_off')
def turn_off():
    print ("turn off")
    stream.stop()
    return ("nothing")

@app.route('/voldn')
def voldn():
    print ("volume down")
    volume.voldn()
    return ("nothing")

@app.route('/volup')
def volup():
    print ("volume up")
    volume.volup()
    return ("nothing")

@app.route('/mute')
def mute():
    print ("mute")
    volume.mute
    return ("nothing")

@app.route('/station/<station_id>')
def station(station_id):
    print (station_id)
    conn = sql.connect("/home/bond/CloudSpeaker/webserver/radio.db")
    conn.row_factory = sql.Row
    cur = conn.cursor()
    cur.execute("select url from stations where id="+station_id+" limit 1;")
    conn.commit()
    rows=cur.fetchall()
    for url in rows:
        print (url[0])
    stream.play(url[0], station_id)
    return ("nothing")

@app.route("/admin")
def admin():
   return render_template('control.html')

if __name__ == "__main__":
   app.run(host='0.0.0.0', port=80, debug=True)

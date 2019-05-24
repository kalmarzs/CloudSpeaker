#!/usr/bin/env python3

from flask import Flask, render_template, request
app = Flask(__name__)
import sqlite3 as sql
import alsaaudio

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
   return render_template("knob.html",rows = rows)


@app.route('/turn_off')
def turn_off():
    print ("turn off")
    return ("nothing")

@app.route('/voldn')
def voldn():
    print ("volume down")
    return ("nothing")

@app.route('/volup')
def volup():
    print ("volume up")
    return ("nothing")

@app.route('/mute')
def mute():
    print ("mute")
    return ("nothing")

@app.route('/station/<station_id>')
def station(station_id):
    print (station_id)
    return ("nothing")

@app.route("/admin")
def admin():
   return render_template('admin.html')

if __name__ == "__main__":
   app.run(host='0.0.0.0', port=80, debug=True)

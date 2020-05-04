#!/usr/bin/env python3
from multiprocessing import Process
from playsound import playsound
import sqlite3 as sql

class Radio(object):
  def __init__(self, id):
    self.id = id

  def start(self):
    global process
    global stid
    stid = self.id
    url = self.get_url()
    process = Process(target=playsound, args=(url,))
    process.start()
    print("started radio program "+ stid)

  def stop(self):
      if 'process' in globals() and process.is_alive():
        print('process is alive')
        process.kill()
        process.join()
        print('process was alive')
      else:
        print('nothing should run')

  def get_url(self):
      conn = sql.connect("/opt/CS2/radio.db")
      conn.row_factory = sql.Row
      cur = conn.cursor()
      cur.execute("select url, name from stations where id="+self.id+" limit 1;")
      conn.commit()
      rows=cur.fetchall()
      for url in rows:
        return(url[0])

  def status(self):
      if 'process' in globals() and 'stid' in globals() and process.is_alive():
        print("status: "+stid)
        return(stid)
      else:
        print("radio is off")
        return("off")
"""
    Copyright 2022 bitValence, Inc.
    All Rights Reserved.

    This program is free software; you can modify and/or redistribute it
    under the terms of the GNU Affero General Public License
    as published by the Free Software Foundation; version 3 with
    attribution addendums as found in the LICENSE.txt.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    Purpose:
      Provide a TPLUG_RPI test environment and serve as an example for
      for how to interface with RPI hardware and communicate with
      TPLUG_RPI
    
    Notes:
      None

"""

import configparser
import socket
import time

import board
from adafruit_lsm6ds.ism330dhcx import ISM330DHCX

config = configparser.ConfigParser()
config.read('tplug_rpi_demo.ini')

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
cfs_ip_addr  = config.get('NETWORK','CFS_IP_ADDR')
cfs_app_port = config.getint('NETWORK','CFS_APP_PORT')
py_app_port  = config.getint('NETWORK','PY_APP_PORT')


i2c = board.I2C()
sensor = ISM330DHCX(i2c)

if __name__ == "__main__":

    loop_delay = config.getint('APP','LOOP_DELAY')
    
    while True:            
        jmsg = 'basecamp/rpi/demo:{"rpi-demo":{"rate-x": %f, "rate-y": %f, "rate-z": %f, "lux": 0}}' % (sensor.gyro)
        print(f'>>> Sending message {jmsg}')
        sock.sendto(jmsg.encode('ASCII'), (cfs_ip_addr, cfs_app_port))
        time.sleep(loop_delay)
    
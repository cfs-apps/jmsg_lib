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
      Provide a JMSG Cmd & TLm python test environment
    
    Notes:
      1. See jmsg_lib/docs/jmsg_topic_plugin_guide.txt for topic
         table configuration and text execution details. 

"""

import configparser
import socket
import threading
import time


config = configparser.ConfigParser()
config.read('jmsg_cmd_tlm.ini')
loop_delay = config.getint('APP','LOOP_DELAY')

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
cfs_ip_addr  = config.get('NETWORK','CFS_IP_ADDR')
cfs_app_port = config.getint('NETWORK','CFS_APP_PORT')
py_app_port  = config.getint('NETWORK','PY_APP_PORT')

    
def tx_thread():
    
    APP_C_DEMO_NOOP = "basecamp/cfs/cmd:185cc0000001007a"
    while True:
        print(f'>>> Sending message {APP_C_DEMO_NOOP}')
        sock.sendto(APP_C_DEMO_NOOP.encode('ASCII'), (cfs_ip_addr, cfs_app_port))
        time.sleep(loop_delay)
        
def rx_thread():

    rx_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    rx_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    rx_socket.bind((cfs_ip_addr, py_app_port))
    rx_socket.setblocking(False)
    rx_socket.settimeout(1000)

    while True:
        print("Pending for recvfrom")
        try:
            while True:
                datagram, host = rx_socket.recvfrom(1024)
                datastr = datagram.decode('utf-8')
                print(f'Received from {host} jmsg size={len(datastr)}: {datastr}')
        except socket.timeout:
            pass
        time.sleep(loop_delay)

    
if __name__ == "__main__":

    rx = threading.Thread(target=rx_thread)
    rx.start()
   
    tx = threading.Thread(target=tx_thread)
    tx.start()
 
    
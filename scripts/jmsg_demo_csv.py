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
      Provide an app that communicates with the JMSG_DEMO cFS app
      using JMSG Comma Separated Varaible (CSV) command & telemetry
      messages
    
    Notes:
      None

"""

import configparser
import socket
import threading
import time
import json
from datetime import datetime

config = configparser.ConfigParser()
config.read('jmsg_demo.ini')

RX_LOOP_DELAY = config.getint('APP','RX_LOOP_DELAY')
TX_LOOP_DELAY = config.getint('APP','TX_LOOP_DELAY')

JMSG_MAX_LEN = config.getint('JMSG','JMSG_MAX_LEN')
JMSG_TOPIC_CSV_CMD_NAME = config.get('JMSG','JMSG_TOPIC_CSV_CMD_NAME')
JMSG_TOPIC_CSV_TLM_NAME = config.get('JMSG','JMSG_TOPIC_CSV_TLM_NAME')

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
CFS_IP_ADDR  = config.get('NETWORK','CFS_IP_ADDR')
CFS_APP_PORT = config.getint('NETWORK','CFS_APP_PORT')
PY_APP_PORT  = config.getint('NETWORK','PY_APP_PORT')


class JmsgDemoCsv():
    """
    """
    def __init__(self):
        self.tx_cnt  = 1
        self.bin_str = ['zero','one'] 
        
    def tx_thread(self):
        """
        Transmit JMSGs from python to the cFS app
        The common use case is to send telemetry message so pressing the
        <Enter> key with no input causes a telemetry message to be sent.
        If any characters are entered then then a JMSG CSV command
        message is sent.
        """
        i = 1
        while True:
            start_time = time.perf_counter()
            input_str = input ("Press <Enter> to send CSV telemetry (send \n\n")
            print(f'input_str: {in_str}, len: {len(input_str)}')
            
            jmsg = ''
            if len(input_str) == 0:
                jmsg = JMSG_TOPIC_CSV_TLM_NAME + f'{{"name": "Demo UDP CSV TLM", "seq-count": {self.tx_cnt}, "date-time": "{datetime.now()}", "parameters": "ex_int,{self.tx_cnt},ex_flt,{float(self.tx_cnt)}"}}'
            else:
                bin_param = self.tx_cnt % 2
                self.bin_str
                jmsg = JMSG_TOPIC_CSV_CMD_NAME + f'{{"name": "Demo UDP CSV CMD", "parameters": "ex_int,{bin_param},ex_flt,{self.bin_str[bin_param]}"}}'         
            
            print(f'>> Sending message {jmsg}\n')
            sock.sendto(jmsg.encode('ASCII'), (CFS_IP_ADDR, CFS_APP_PORT))
            time.sleep(TX_LOOP_DELAY)
            self.tx_cnt += 1

            
    def rx_thread(self):

        rx_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        rx_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        rx_socket.bind((CFS_IP_ADDR, PY_APP_PORT))
        rx_socket.setblocking(False)
        rx_socket.settimeout(1000)

        while True:
            jmsg = None
            try:
                while True:
                    print("***** Pending for JMSG_DEMO command ...")
                    datagram, host = rx_socket.recvfrom(JMSG_MAX_LEN)
                    if datagram:
                        jmsg = datagram
                    if jmsg:
                        jmsg_str = jmsg.decode('utf-8')
                        print(f'Received from {host} JMSG {len(jmsg_str)}: {jmsg_str}')
                        jmsg_str = jmsg_str.replace("\x00", "").replace("\x01", "")
                        process_command(jmsg_str)
                    print('*****\n')
                    time.sleep(RX_LOOP_DELAY)                
            except socket.timeout:
                pass

    def process_command(self, jmsg_str):

        try:
            # Text following prefix is assumed to be JSON message 
            if jmsg_str.startswith(JMSG_TOPIC_CSV_CMD_NAME):
                json_str = jmsg_str.replace(JMSG_TOPIC_CSV_CMD_NAME, "")
                json_str = json_str.replace('\n','\\n')
                print(f'json_str: {json_str}')
            else:
                print(f'Received JMSG not addressed to jmsg_demo_csv. Expected {JMSG_TOPIC_CSV_CMD_NAME}')
        except Exception as e:
            print(f'Process request exception: {e}')

    def start(self):
        
        #self.tx = threading.Thread(target=self.tx_thread)
        #self.tx.start()

        self.rx = threading.Thread(target=self.rx_thread)
        self.rx.start()


if __name__ == "__main__":

    jmsg_demo_csv = JmsgDemoCsv()
    jmsg_demo_csv.start()

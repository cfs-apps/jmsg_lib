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
      Provide an environment to test basic functionality of the 
      JMSG script_cmd, csv_cmd and csv_tlm topics
    
    Notes:
      1. Only one topic can be tested at a time that is defined
         by the INI file USE_CASE parameter
      2. Use case tests verify JMSG topic messages. These messages
         are designed for end users to customize content such as 
         telemetry parameters. That verification is performed by the
         end user app.

"""

import configparser
import socket
import threading
import time
import json


config = configparser.ConfigParser()
config.read('jmsg_demo.ini')

USE_CASE = config.get('APP','USE_CASE')
RX_LOOP_DELAY = config.getint('APP','RX_LOOP_DELAY')
TX_LOOP_DELAY = config.getint('APP','TX_LOOP_DELAY')

JMSG_MAX_LEN = config.getint('JMSG','JMSG_MAX_LEN')
JMSG_TOPIC_SCR_CMD_NAME = config.get('JMSG','JMSG_TOPIC_SCR_CMD_NAME')
JMSG_TOPIC_CSV_CMD_NAME = config.get('JMSG','JMSG_TOPIC_CSV_CMD_NAME')
JMSG_TOPIC_CSV_TLM_NAME = config.get('JMSG','JMSG_TOPIC_CSV_TLM_NAME')
JMSG_TOPIC_CSV_RPI_NAME = config.get('JMSG','JMSG_TOPIC_CSV_RPI_NAME')
RUN_SCRIPT_TEXT_CMD     = config.getint('JMSG','RUN_SCRIPT_TEXT_CMD')
RUN_SCRIPT_FILE_CMD     = config.getint('JMSG','RUN_SCRIPT_FILE_CMD')

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
CFS_IP_ADDR  = config.get('NETWORK','CFS_IP_ADDR')
CFS_APP_PORT = config.getint('NETWORK','CFS_APP_PORT')
PY_APP_PORT  = config.getint('NETWORK','PY_APP_PORT')


def tx_thread():
    
    i = 1
    while True:
        cont = input ("Enter to send")
        jmsg = ''
        if USE_CASE == 'SCRIPT_CMD':
            jmsg = JMSG_TOPIC_SCR_CMD_NAME + "{\"command\": 1, \"script-file\": \"Undefined\", \"script-text\": \"print('Hello world')\"}"
        elif USE_CASE == 'CSV_CMD':
            jmsg = JMSG_TOPIC_CSV_CMD_NAME + "{\"name\": \"UDP CSV CMD\", \"parameters\": \"None\"}"
        elif USE_CASE == 'CSV_TLM':
            jmsg = JMSG_TOPIC_CSV_TLM_NAME + "{\"name\": \"UDP CSV TLM\", \"seq-count\": 99, \"date-time\": \"0\", \"parameters\": \"None\"}"
        elif USE_CASE == 'CSV_RPI':
            floati = float(i)
            jmsg = JMSG_TOPIC_CSV_CMD_NAME + '{"name":"UDP RPI", "parameters": "rate-x,%f,rate-y,%f,rate-z,%f,lux,%i"}' % (floati*1.0,floati*2.0,floati*3.0,i)
        
        print(f'>>> Sending message {jmsg}')
        sock.sendto(jmsg.encode('ASCII'), (CFS_IP_ADDR, CFS_APP_PORT))
        time.sleep(TX_LOOP_DELAY)
        i += 1

        
def rx_thread():

    rx_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    rx_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    rx_socket.bind((CFS_IP_ADDR, PY_APP_PORT))
    rx_socket.setblocking(False)
    rx_socket.settimeout(1000)

    while True:
        jmsg = None
        print("Pending for recvfrom")
        try:
            while True:
                datagram, host = rx_socket.recvfrom(JMSG_MAX_LEN)
                if datagram:
                    jmsg = datagram
                if jmsg:
                    jmsg_str = jmsg.decode('utf-8')
                    print(f'*****\nReceived from {host} JMSG {len(jmsg_str)}: {jmsg_str}\n')
                    jmsg_str = jmsg_str.replace("\x00", "").replace("\x01", "")
                    if USE_CASE == 'SCRIPT_CMD':
                        process_scr_cmd_jmsg(jmsg_str)
                    elif USE_CASE == 'CSV_CMD' or USE_CASE == 'CSV_RPI':
                        process_csv_cmd_jmsg(jmsg_str)
                    elif USE_CASE == 'CSV_TLM':
                        process_csv_tlm_jmsg(jmsg_str)
                        
        except socket.timeout:
            pass
        print('*****\n\n')
        time.sleep(RX_LOOP_DELAY)


def process_scr_cmd_jmsg(jmsg_str):

    try:
        # Text following prefix is assumed to be JSON message 
        if jmsg_str.startswith(JMSG_TOPIC_SCR_CMD_NAME):
            json_str = jmsg_str.replace(JMSG_TOPIC_SCR_CMD_NAME, "")
            json_str = json_str.replace('\n','\\n')
            json_dict = json.loads(json_str)
            command = json_dict["command"]
            if command == RUN_SCRIPT_TEXT_CMD:
                #print(f'>>json_dict: {json_dict}\n')
                exec(json_dict["script-text"])
                print("")                
            elif command == RUN_SCRIPT_FILE_CMD:
                exec(open(json_dict["script-file"]).read())
            else:
                print(f'Received JMSG with invalid command {command}')
        else:
            print(f'Received JMSG not addressed to JMSG Demo. Expected {JMSG_TOPIC_SCR_CMD_NAME}')
    except Exception as e:
        print(f'Process script command exception: {e}\n')

def process_csv_cmd_jmsg(jmsg_str):
    print("process_csv_cmd_jmsg()")

def process_csv_tlm_jmsg(jmsg_str):
    print("process_csv_tlm_jmsg()")


if __name__ == "__main__":

    rx = threading.Thread(target=rx_thread)
    rx.start()
   
    tx = threading.Thread(target=tx_thread)
    tx.start()
 
    #process_jmsg(TEST1_JMSG)
    #process_jmsg(TEST2_JMSG)
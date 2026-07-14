import serial
import time
import sys
import datetime
import os

# 通訊參數設定
COM_PORT = 'COM3'      # HC-12 接收器的 COM Port
BAUD_RATE = 9600        # esp32 serial2 baud rate 
SENSOR_COUNT = 8        # 感測器數量

# timestamp
mission_time = time.strftime('%Y-%m-%d_%H:%M:%S',time.localtime())
filename = time.strftime('%Y_%m_%d_%H_%M',time.localtime())

# folder setup
loc = os.getcwd()
data_folder = "data_logs"
data_folder_path = os.path.join(loc, data_folder)
data_folder_path = os.path.join(data_folder_path, f"data_{filename}")
os.makedirs(data_folder_path, exist_ok=True) 

# haedar format
def get_nasa_header(channel_idx):
    return f"""
==============================================================================
                         MAGNETOMETER TELEMETRY DATA         
==============================================================================
FORMAT       : ASCII
SENSOR       : QMC5883P (Channel {channel_idx})
DATA_UNIT    : Gauss
MISSION_TIME : {mission_time}
END_OF_HEADER
PC_TIMESTAMP              ESP32_MS          X          Y          Z
==============================================================================
"""

# store file for each channel
file_handles = []

try:
    # start serial comm
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)

    ser.reset_input_buffer()

    print(f" connection success({COM_PORT} @ {BAUD_RATE} baud)！\n")
    print(f" setting up data file\n")

    # create 8 files for 8 channels
    for i in range(SENSOR_COUNT):
        fname = f"MAG_CH{i}_{filename}.txt"
        file_location = os.path.join(data_folder_path, fname)
        
        f = open(file_location, 'w', encoding='utf-8') # open(fiename,'mode(read,write,append),encoding')
        f.write(get_nasa_header(i))
        file_handles.append(f) # add to file storage list
        print(f"channel {i} ready...\n")

    print("\n press ctrl+c to stop receiving data\n")
    

    # while loop to keep reading serial data
    while True:
        if ser.in_waiting > 0:

            raw_line = ser.readline().decode('utf-8', errors='ignore').strip()
            raw_line = raw_line.replace('\x00', '').strip()

            if not raw_line:
                continue

            # split data by ","
            data = raw_line.split(',')

            # check if data length match
            # time + (x,y,z)*8 = 1 + (8*3)
            expected_length = 1 + (SENSOR_COUNT * 3)
            
            if len(data) == expected_length:

                now = datetime.datetime.now(tz=datetime.timezone(datetime.timedelta(hours=8)))
                pc_time = now.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3] # timestamp with milliseconds precision on pc

                time_ms = data[0] # timestamp in milliseconds from ESP32

                for i in range(SENSOR_COUNT):
                    # CH0 (i=0): index 1, 2, 3
                    # CH1 (i=1): index 4, 5, 6
                    # CH2 (i=2): index 7, 8, 9
                    # CH3 (i=3): index 10, 11, 12
                    # CH4 (i=4): index 13, 14, 15
                    # CH5 (i=5): index 16, 17, 18
                    # CH6 (i=6): index 19, 20, 21
                    # CH7 (i=7): index 22, 23, 24
                    idx = 1 + (i * 3)
                    x, y, z = data[idx], data[idx+1], data[idx+2]

                    # write to file with formatted output
                    formatted_line = f" {pc_time} | {time_ms:<10} {x:>8} {y:>8} {z:>8}\n"
                    file_handles[i].write(formatted_line)
                    file_handles[i].flush() 

                # receiving status print
                print(f"[時間: {pc_time:>6}] data received！ {SENSOR_COUNT} sensors' data written in file.\n")
            
            else:
                # packet loss
                print(f"data packet lossed: {raw_line}\n")

except serial.SerialException:
    print(f"\n error：failed to connect {COM_PORT}！")
except KeyboardInterrupt:
    print(f"\nmission end.\n")
finally:
    # end of comm save files
    for i, f in enumerate(file_handles):
        if not f.closed:
            f.close()
            print(f"[CH{i}] file closed and saved.\n")
    if 'ser' in locals() and ser.is_open:
        ser.close()
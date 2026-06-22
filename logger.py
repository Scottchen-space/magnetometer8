import serial
import time

COM_PORT = 'COM3' # 記得確認你的 COM Port
BAUD_RATE = 115200

filename = f"MAG_DATA_{time.strftime('%Y%m%d_%H%M%S')}.txt"

# 預先在 Python 裡把 Header 寫好 (使用多行字串包裝)
NASA_HEADER = """=========================================
               MAGNETOMETER              
=========================================
FORMAT       : ASCII
SENSOR       : QMC5883L
DATA_UNIT    : Gauss
END_OF_HEADER
TIME_MS           X         Y          Z
"""

try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    print(f"✅ 連線成功！正在將數據寫入 {filename}")
    print(f"🛑 按下鍵盤 Ctrl + C 停止記錄\n")

    # 開啟 txt 檔案準備寫入
    with open(filename, 'w', encoding='utf-8') as f:
        
        # 1. 檔案一打開，Python 先自己把 Header 寫進 txt 檔，並印在螢幕上
        f.write(NASA_HEADER)
        print(NASA_HEADER, end="") 
        
        # 2. 開始進入無限迴圈，接 ESP32 丟過來的純數字
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').rstrip()
                
                # 如果讀到空行或亂碼就跳過
                if not line:
                    continue
                    
                print(line)          # 顯示在螢幕上
                f.write(line + '\n') # 寫進檔案裡
                f.flush()            # 強制存檔

except serial.SerialException:
    print(f"\nerror!")
except KeyboardInterrupt:
    print(f"\n🛑 記錄結束，檔案已安全儲存。")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
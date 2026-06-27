import serial
import time

COM_PORT = 'COM7' # 如果你的 COM Port 改變了請改這裡
BAUD_RATE = 9600  # 必須與 ESP32 端的 Serial2 完全一致

filename = f"MAG_DATA_{time.strftime('%Y%m%d_%H%M%S')}.txt"

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
    # 建立連線
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    print(f"✅ 連線成功！正在將數據寫入 {filename}")
    print(f"🛑 按下鍵盤 Ctrl + C 停止記錄\n")

    with open(filename, 'w', encoding='utf-8') as f:
        f.write(NASA_HEADER)
        print(NASA_HEADER, end="") 
        
        while True:
            if ser.in_waiting > 0:
                # 讀取並解碼，忽略無法解析的亂碼
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if not line:
                    continue
                    
                print(line)          
                f.write(line + '\n') 
                f.flush()            

except serial.SerialException:
    print(f"\n❌ 錯誤：無法開啟 {COM_PORT}，請檢查藍牙是否連線或 COM Port 數字是否正確！")
except KeyboardInterrupt:
    print(f"\n🛑 記錄結束，檔案已安全儲存。")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
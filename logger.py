import serial
import time
import sys

# === 遙測通訊參數設定 ===
COM_PORT = 'COM3'      # 依你電腦實際連接 HC-12 轉接板的 COM Port 修改
BAUD_RATE = 9600        # 必須與 ESP32 端的 Serial2 完全一致
SENSOR_COUNT = 4        # 感測器數量 (對應 CH0 ~ CH3)

# 產生這回任務的共用時間戳記，確保所有檔案都有相同的建檔時間標記
mission_time = time.strftime('%Y%m%d_%H%M%S')

# 💡 NASA 風格標頭產生器 (動態填入通道編號與最新 QMC5883P 型號)
def get_nasa_header(channel_idx):
    return f"""=========================================
           MAGNETOMETER TELEMETRY DATA         
=========================================
FORMAT       : ASCII
SENSOR       : QMC5883P (Channel {channel_idx})
DATA_UNIT    : Gauss
MISSION_TIME : {mission_time}
END_OF_HEADER
TIME_MS            X        Y        Z
=========================================
"""

# 用來存放 4 個檔案控制物件的串列
file_handles = []

try:
    # 1. 建立無線電序列埠連線
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    print(f"✅ 射頻遙測連線成功 ({COM_PORT} @ {BAUD_RATE} baud)！")
    print(f"🚀 正在為 {SENSOR_COUNT} 顆感測器建立獨立的數據存檔...\n")

    # 2. 一口氣打開 4 個獨立的檔案，並依序寫入各自的 NASA 標頭
    for i in range(SENSOR_COUNT):
        fname = f"MAG_CH{i}_{mission_time}.txt"
        f = open(fname, 'w', encoding='utf-8')
        f.write(get_nasa_header(i))
        file_handles.append(f)
        print(f"   📁 [通道 {i} 準備就緒] 檔案建立: {fname}")

    print("\n🛑 按下鍵盤 Ctrl + C 可隨時停止遙測記錄\n")
    print("-" * 65)
    print("即時遙測接收監控台 (Telemetry Monitor):")
    print("-" * 65)

    # 3. 進入無窮迴圈，持續監聽天空傳來的電波封包
    while True:
        if ser.in_waiting > 0:
            # 讀取一整行封包並解碼，忽略無線電空中雜訊產生的亂碼
            raw_line = ser.readline().decode('utf-8', errors='ignore').strip()
            
            if not raw_line:
                continue

            # 💡 步驟一：用逗號將這一長串字串切成串列 (List)
            data = raw_line.split(',')

            # 💡 步驟二：航太級封包完整性檢查 (Defensive Check)
            # 正常長度必須是： 1個時間 + (4顆 * 3軸) = 13 個欄位
            expected_length = 1 + (SENSOR_COUNT * 3)
            
            if len(data) == expected_length:
                time_ms = data[0] # 取出第一欄的時間戳記

                # 💡 步驟三：迴圈分發！將數據送到各自專屬的檔案裡
                for i in range(SENSOR_COUNT):
                    # 數學映射：計算第 i 顆感測器的 X, Y, Z 在 data 串列裡的 Index
                    # CH0 (i=0): index 1, 2, 3
                    # CH1 (i=1): index 4, 5, 6
                    # CH2 (i=2): index 7, 8, 9
                    # CH3 (i=3): index 10, 11, 12
                    idx = 1 + (i * 3)
                    x, y, z = data[idx], data[idx+1], data[idx+2]

                    # 格式化寫入（使用 <10 與 >8 讓數字排版像是精密的報表）
                    formatted_line = f"{time_ms:<10} {x:>8} {y:>8} {z:>8}\n"
                    file_handles[i].write(formatted_line)
                    file_handles[i].flush() # ⚡ 立即寫入硬碟！防止突然斷電導致資料遺失

                # 在終端機印出漂亮簡潔的接收成功狀態
                print(f"⚡ [時間: {time_ms:>6} ms] 完美解包！ {SENSOR_COUNT} 顆感測器數據已同步寫入檔案。")
            
            else:
                # 如果因為空中電波干擾導致封包斷裂（例如只收到一半），發出黃字警告但不會當機！
                print(f"⚠️ [電波雜訊丟包] 收到不完整封包 (欄位數 {len(data)}/{expected_length}): {raw_line}")

except serial.SerialException:
    print(f"\n❌ 錯誤：無法開啟 {COM_PORT}！請檢查 HC-12 接收器是否插好，或 COM Port 數字是否正確。")
except KeyboardInterrupt:
    print(f"\n🛑 接收任務手動終止。")
finally:
    # 4. 安全收尾：確保程式結束時，4 個檔案都有被妥善關閉保存
    for i, f in enumerate(file_handles):
        if not f.closed:
            f.close()
            print(f"🔒 [CH{i}] 檔案已安全封存。")
    if 'ser' in locals() and ser.is_open:
        ser.close()
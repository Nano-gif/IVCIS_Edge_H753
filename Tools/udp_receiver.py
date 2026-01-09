#!/usr/bin/env python3
"""
IVCIS UDP JPEG 接收器
用法: python udp_receiver.py
功能: 接收 STM32 发送的 UDP 分片，重组为 JPEG 文件
"""
import socket
import os
from datetime import datetime

UDP_IP = "0.0.0.0"  # 监听所有网卡
UDP_PORT = 8080
SAVE_DIR = "./received_images"
os.makedirs(SAVE_DIR, exist_ok=True)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
print(f"[UDP Receiver] Listening on {UDP_IP}:{UDP_PORT}")

jpeg_buffer = bytearray()
frame_count = 0

while True:
    data, addr = sock.recvfrom(1500)
    jpeg_buffer.extend(data)
    
    # 检测 JPEG 结束标记 (FF D9)
    if len(jpeg_buffer) >= 2 and jpeg_buffer[-2:] == b'\xff\xd9':
        frame_count += 1
        filename = f"{SAVE_DIR}/frame_{frame_count:04d}_{datetime.now().strftime('%H%M%S')}.jpg"
        with open(filename, "wb") as f:
            f.write(jpeg_buffer)
        print(f"[OK] Saved {filename} ({len(jpeg_buffer)} bytes)")
        jpeg_buffer.clear()

import socket
import cv2
import numpy as np
import os

# ==========================================
# IVCIS Edge H753 Visualizer
# ==========================================

UDP_IP = "0.0.0.0" 
UDP_PORT = 8080
MAGIC = b'IVCI'

print(f"🚀 IVCIS Visualizer Starting on port {UDP_PORT}...")
print("Waiting for packets... (Ensure STM32 IP is 192.168.1.10 and Laptop is 192.168.1.100)")

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((UDP_IP, UDP_PORT))

buffer = {} # {frame_id: [chunk0, chunk1, ...]}

try:
    while True:
        data, addr = sock.recvfrom(2048)
        
        # Parse IVCI Header (20 Bytes)
        # 0-4: Magic, 4-8: FrameID, 8-10: Cidx, 10-12: Ccnt, 12-16: Total, 16-20: Flags/Res
        if len(data) > 20 and data[:4] == MAGIC:
            frame_id = int.from_bytes(data[4:8], 'little')
            idx = int.from_bytes(data[8:10], 'little')
            cnt = int.from_bytes(data[10:12], 'little')
            
            # [LOG] Track incoming chunks
            if idx == 0:
                print(f"--- Frame #{frame_id} started ({cnt} chunks expected) ---")
            
            if frame_id not in buffer:
                buffer[frame_id] = [None] * cnt
            
            # Boundary check
            if idx < cnt:
                buffer[frame_id][idx] = data[20:]
                # print(f"  [.] Chunk {idx}/{cnt-1} received")
            
            # Check if frame is complete
            if all(x is not None for x in buffer[frame_id]):
                print(f"✅ Frame #{frame_id} COMPLETE. Decoding...")
                img_data = b''.join(buffer[frame_id])
                nparr = np.frombuffer(img_data, np.uint8)
                img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
                
                if img is not None:
                    # FPS & Info Overlay
                    cv2.putText(img, f"Frame: {frame_id} | Chunks: {cnt}", (10, 30), 
                                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                    cv2.imshow('IVCIS_Edge_H753_Live', img)
                
                # Cleanup old frames to prevent memory leak
                del buffer[frame_id]
                # Keep buffer small
                if len(buffer) > 10:
                    oldest = min(buffer.keys())
                    del buffer[oldest]

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    print("\nStopping... 喵~")
finally:
    sock.close()
    cv2.destroyAllWindows()

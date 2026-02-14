import socket
import threading

# ================== CONFIGURATION ==================
STM32_IP = "192.168.1.10"    # STM32 IP
STM32_PORT = 5005            # STM32 UDP port
LOCAL_PORT = 5006            # local port to receive responses
BUFFER_SIZE = 1024
# ===================================================

stop_event = threading.Event()

def udp_receiver(sock):
    """Thread that listens for incoming UDP messages."""
    while not stop_event.is_set():
        try:
            sock.settimeout(1.0)
            data, addr = sock.recvfrom(BUFFER_SIZE)
            print(f"[RX from {addr}]: {data.decode(errors='ignore')}")
            print("> ", end="", flush=True)
        except socket.timeout:
            continue
        except OSError as e:
            print(f"\n[Receiver error]: {e}")
            break

def udp_sender(sock, target_addr):
    """Thread that sends user input via UDP."""
    message = input('> ')
    while True:
        try:
            if message.lower() in ("exit", "quit"):
                print("Exiting...")
                stop_event.set()
                break

            sock.sendto(message.encode(), target_addr)

            message = input()

        except Exception as e:
            print(f"[Sender error]: {e}")
            break

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("", LOCAL_PORT))

    target_addr = (STM32_IP, STM32_PORT)

    print("UDP client started")
    print(f"Sending to {STM32_IP}:{STM32_PORT}")
    print("Type messages and press Enter (type 'exit' to quit)\n")

    rx_thread = threading.Thread(target=udp_receiver, args=(sock,), daemon=True)
    tx_thread = threading.Thread(target=udp_sender, args=(sock, target_addr))

    rx_thread.start()
    tx_thread.start()

    tx_thread.join()
    rx_thread.join()
    sock.close()

if __name__ == "__main__":
    main()

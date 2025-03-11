
# Custom STUN server to manage the initial communication between peers
from concurrent.futures import ThreadPoolExecutor
import socket
import time

guests = {} # The "guest book" (dictionary)
TIMEOUT = 30 # Time until forgetting a client (seconds)
SERVER_IP = "127.0.0.1"
SERVER_PORT = 12345

def handle_client(data, client_address, server_socket):
    message = data.decode()
    parts = message.split(":")

    if parts[0] == "HEARTBEAT":
        # A client is letting us know they are still online.
        if client_address in guests:
            guests[client_address]["last_seen"] = time.time()
        return

    elif parts[0] == "JOIN":
        guests[client_address] = {"last_seen": time.time()}
        print(f"Client joined: {client_address}")
        response = "JOIN_OK".encode()

    elif parts[0] == "LIST":
        formatted_guestlist = ""
        now = time.time()
        dead_clients = []

        for address, guest_info in guests.items():
            if now - guest_info["last_seen"] > TIMEOUT:
                dead_clients.append(address)
            else:
                formatted_guestlist += f"{address[0]}:{address[1]};"
        for address in dead_clients: # Remove dead peers
            del guests[address]
            print(f"Client timed out: {address}")

        response = formatted_guestlist.encode()

    server_socket.sendto(response, client_address)
    
def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_address = (SERVER_IP, SERVER_PORT)
    server_socket.bind(server_address)
    print("Server started, listening on port " + str(SERVER_PORT))

    with ThreadPoolExecutor(max_workers=5) as executor:
        while True:
            data, client_address = server_socket.recvfrom(4096)
            executor.submit(handle_client, data, client_address, server_socket)

if __name__ == "__main__":
    main()

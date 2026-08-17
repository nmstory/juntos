
# Custom STUN server to manage the initial communication between peers
import asyncio
import os
import sys
import time

# Flush prints line-by-line even when stdout is piped/redirected (e.g. over SSH)
sys.stdout.reconfigure(line_buffering=True)

guests = {} # The "guest book" (dictionary)
TIMEOUT = 30 # Time until forgetting a client (seconds)
SERVER_IP = os.environ.get("STUN_HOST", "0.0.0.0")
SERVER_PORT = int(os.environ.get("STUN_PORT", "12345"))

class StunProtocol(asyncio.DatagramProtocol):
    def connection_made(self, transport):
        self.transport = transport

    def datagram_received(self, data, client_address):
        message = data.decode()
        parts = message.split(":")

        if parts[0] == "HEARTBEAT":
            # A client is letting us know they are still online.
            if client_address in guests:
                guests[client_address]["last_seen"] = time.time()
            return

        elif parts[0] == "JOIN":
            now = time.time()
            # Peers already in the guest book, about to be introduced to the
            # newcomer. This avoids introducing it to itself.
            live_peers = [addr for addr, info in guests.items()
                          if now - info["last_seen"] <= TIMEOUT]

            guests[client_address] = {"last_seen": now}
            print(f"Client joined: {client_address}")

            # Tell each peer already waiting about the newcomer, so they PING it immediately
            new_peer_notice = f"NEW_PEER:{client_address[0]}:{client_address[1]}".encode()
            for addr in live_peers:
                self.transport.sendto(new_peer_notice, addr)
                print(f"Notified {addr} about new peer {client_address}")

            response = "JOIN_OK".encode()

        elif parts[0] == "LIST":
            if len(guests) == 1:
                response = "EMPTY".encode()
            else:
                formatted_guestlist = ""
                now = time.time()
                dead_clients = []

                for address, guest_info in guests.items():
                    if now - guest_info["last_seen"] > TIMEOUT:
                        dead_clients.append(address)
                    else:
                        if address != client_address: # Don't send the client their own address
                            formatted_guestlist += f"{address[0]}:{address[1]};"
                for address in dead_clients: # Remove dead peers
                    del guests[address]
                    print(f"Client timed out: {address}")

                response = formatted_guestlist.encode()

        self.transport.sendto(response, client_address)

async def main():
    loop = asyncio.get_running_loop()
    transport, _ = await loop.create_datagram_endpoint(
        StunProtocol,
        local_addr=(SERVER_IP, SERVER_PORT),
    )
    print("Server started, listening on port " + str(SERVER_PORT))

    try:
        await asyncio.Future() # run forever
    finally:
        transport.close()

if __name__ == "__main__":
    asyncio.run(main())

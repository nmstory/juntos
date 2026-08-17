#!/usr/bin/env bash
# Capture the packets that prove hole punching happened: the JOIN/LIST
# exchange with the rendezvous server, the PING/PONG that opens each NAT's
# mapping, and (if it works) the chat traffic flowing peer-to-peer afterward.
#
# Run this on EACH machine involved in the test (the rendezvous VM and both
# chat clients) a few seconds before you start juntos_chat, and stop it
# (Ctrl-C) a few seconds after you've exchanged messages.
#
# Usage: sudo ./capture.sh [output.pcap] [port]
set -euo pipefail

OUT="${1:-capture-$(hostname)-$(date +%s).pcap}"
PORT="${2:-12345}"

IFACE="${IFACE:-any}"

echo "Capturing UDP traffic on port ${PORT} (and the client's own ephemeral"
echo "port range, since juntos peers talk on whatever port you pass as"
echo "DESIRED_PORT_NUMBER) -- interface: ${IFACE}"
echo "Writing to: ${OUT}"
echo "Press Ctrl-C when you're done exchanging chat messages."
echo

# udp port PORT catches STUN traffic; 'or udp' is intentionally broad here
# since peer-to-peer traffic uses whatever port you launched juntos_chat on.
# Narrow this with -w + a display filter later in Wireshark if it's noisy.
sudo tcpdump -i "${IFACE}" -w "${OUT}" udp

echo
echo "Saved ${OUT}. Inspect with:"
echo "  tcpdump -r ${OUT} -v"
echo "  wireshark ${OUT}"
echo
echo "What to look for:"
echo "  1. This host -> STUN server: 'JOIN' then 'LIST:'"
echo "  2. STUN server -> this host: 'JOIN_OK' then the peer's ip:port"
echo "  3. This host <-> peer (NOT via the STUN server): 'PING' then 'PONG'"
echo "     -- this is the hole-punch. It's the first packet either NAT lets"
echo "        through from the peer's IP, before the peer has sent anything"
echo "        to this host's public mapping."
echo "  4. This host <-> peer: your typed chat text, flowing directly"
echo "     (confirm the STUN server's pcap shows nothing after step 2 --"
echo "     proof the rendezvous server dropped out of the path)."
echo "  5. This host -> STUN server: 'HEARTBEAT:' every ~5s (network_handler.h"
echo "     TIME_BETWEEN_HEARTBEATS) keeping the NAT mapping + registration alive."

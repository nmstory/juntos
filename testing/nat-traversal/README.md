# Real cross-NAT hole-punch test

Goal: prove the JOIN/LIST → PING/PONG → direct-traffic flow in
[`stun.py`](../../stun.py) and
[`src/session_linux.cpp`](../../src/session_linux.cpp) actually punches
through two independent, real NATs — not just loopback — and capture the
packets as evidence for the blog post.

## What you need

- A DigitalOcean account (or any cloud provider — steps below use `doctl`
  and the DO console, swap for your provider's equivalent).
- **Two separate physical machines**, each able to build and run
  `juntos_chat`, each on a genuinely different network:
  - Peer A: this laptop on home wifi.
  - Peer B: a second device tethered to a phone's cellular hotspot (a second
    laptop, a Raspberry Pi, a WSL box — anything that isn't sharing Peer A's
    router). Carrier cellular NAT is usually **carrier-grade NAT (CGNAT)**,
    which is a good stress test since it's stricter than a typical home
    router's NAT.
- `tcpdump` on all three hosts (VM, Peer A, Peer B). Installed automatically
  on the VM by the provisioning script below; on macOS it's preinstalled,
  on Linux `apt install tcpdump` / `pacman -S tcpdump` etc.

## 1. Create the rendezvous VM

Cheapest droplet is plenty — this is a UDP echo/guestbook server, not a
compute job.

Via `doctl` (if you have it installed and authenticated):
```sh
doctl compute droplet create juntos-stun \
  --region nyc1 \
  --image ubuntu-24-04-x64 \
  --size s-1vcpu-512mb-10gb \
  --ssh-keys <your-ssh-key-fingerprint> \
  --wait
```

Or via the [DigitalOcean console](https://cloud.digitalocean.com/droplets/new):
Ubuntu 24.04, cheapest "Basic" plan, add your SSH key, create.

Note the droplet's public IP once it's up (`doctl compute droplet list` or
the console).

## 2. Provision it

```sh
scp provision_stun_vm.sh root@<DROPLET_IP>:~
ssh root@<DROPLET_IP> './provision_stun_vm.sh'
```

This installs Python 3 + tcpdump, clones the repo for `stun.py`, opens UDP
12345 in `ufw`, and runs `stun.py` as a systemd service (`juntos-stun`) so
it keeps running after you disconnect. Confirm it's up:

```sh
ssh root@<DROPLET_IP> 'systemctl status juntos-stun --no-pager'
```

## 3. Build juntos_chat on both peer machines

On each of Peer A and Peer B:
```sh
git clone https://github.com/nmstory/juntos.git
cd juntos
mkdir build && cd build
cmake -D JUNTOS_BUILD_EXAMPLES=ON .. && make
```

## 4. Start captures (all three hosts)

Copy `capture.sh` to the VM and to both peers. Start it on **all three**
a few seconds before step 5:

```sh
# on the VM
sudo ./capture.sh stun-server.pcap

# on Peer A
sudo ./capture.sh peer-a.pcap

# on Peer B
sudo ./capture.sh peer-b.pcap
```

## 5. Run the chat demo across real networks

On Peer A:
```sh
./examples/chat/juntos_chat 9000 <DROPLET_IP> 12345
```

On Peer B (a minute or so later, so you can see the `LIST` response
actually return a peer instead of `EMPTY`):
```sh
./examples/chat/juntos_chat 9001 <DROPLET_IP> 12345
```

Watch for `Peer discovered: <ip>:<port>` on both sides — that's the
PING/PONG handshake completing. Type a few messages back and forth. Let it
run ~30s so you also catch a `HEARTBEAT:` cycle (every 5s, see
`TIME_BETWEEN_HEARTBEATS` in
[`include/network_handler.h`](../../include/network_handler.h)).

Then Ctrl-C the chat clients and the three `capture.sh` runs.

## 6. Confirm it actually punched through

Pull the three pcaps to one machine and check, in order:

1. **stun-server.pcap**: both peers' `JOIN`, the `LIST:` request/response
   pair, and periodic `HEARTBEAT:` — but **nothing after the initial
   handshake that looks like chat text**. If chat text shows up in this
   capture, traffic is still relaying through the server and the hole punch
   failed.
2. **peer-a.pcap** / **peer-b.pcap**: `PING` and `PONG` packets exchanged
   directly between Peer A's public `ip:port` and Peer B's public `ip:port`
   — addresses that came out of the STUN server's `LIST` response, not the
   droplet's IP. This is the actual hole-punch evidence.
3. Chat text in peer-a.pcap / peer-b.pcap going straight between the two
   peer IPs, with no droplet IP involved at that point.

`wireshark stun-server.pcap` (or `tcpdump -r ... -A` to see payloads as
text, since these are unencrypted ASCII protocol messages) makes this easy
to eyeball. A filter like `udp.port == 12345` on the server capture and
`udp` on the peer captures is enough — there isn't much other traffic to
wade through.

## 7. Tear down

Destroy the droplet once you've pulled the pcaps — no reason to keep paying
for it:
```sh
doctl compute droplet delete juntos-stun
```

## Troubleshooting

- **`LIST` keeps returning `EMPTY`**: peers must both have hit `JOIN` within
  the `TIMEOUT` window in `stun.py` (30s) of each other, and the `LIST` call
  happens once at startup in `initSessionToStun` — if Peer A joined more
  than 30s before Peer B, Peer A may have been forgotten. Restart Peer A's
  client after Peer B is up.
- **No `PING`/`PONG` between peers, only through the server**: likely a
  symmetric NAT on one side (common on some corporate networks, less common
  on home routers and most cellular carriers, but it happens). This is
  useful negative data for the post too — note which network exhibited it.
- **`ufw` blocks the return traffic**: `provision_stun_vm.sh` opens
  `12345/udp` inbound; outbound is allowed by default under ufw, so this
  usually isn't the issue, but double check `ufw status` if the VM never
  sees a `JOIN`.

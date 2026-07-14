#!/usr/bin/env bash
# Run this ON the rendezvous VM (as root or via sudo) after first boot.
# Target: a fresh Ubuntu 24.04 droplet. Installs deps, opens the STUN UDP
# port, and runs stun.py as a systemd service so it survives your SSH
# session ending and reboots.
#
# Note: stun.py's listening port is a hardcoded constant (SERVER_PORT in
# stun.py), not a CLI arg -- so this script doesn't take a port parameter.
# If you need a different port, edit SERVER_PORT in stun.py before running
# this, or after cloning into INSTALL_DIR below.
#
# Usage: sudo ./provision_stun_vm.sh
set -euo pipefail

PORT=12345  # must match SERVER_PORT in stun.py
JUNTOS_REPO="https://github.com/nmstory/juntos.git"
INSTALL_DIR="/opt/juntos"

echo "== Installing dependencies =="
apt-get update -y
apt-get install -y python3 git ufw tcpdump

echo "== Cloning juntos (for stun.py) =="
if [ -d "$INSTALL_DIR" ]; then
  git -C "$INSTALL_DIR" pull
else
  git clone --depth 1 "$JUNTOS_REPO" "$INSTALL_DIR"
fi

echo "== Opening UDP $PORT in ufw =="
ufw allow "${PORT}/udp"
ufw allow OpenSSH
ufw --force enable

echo "== Installing systemd unit =="
cat > /etc/systemd/system/juntos-stun.service <<EOF
[Unit]
Description=Juntos rendezvous (STUN) server
After=network.target

[Service]
ExecStart=/usr/bin/python3 ${INSTALL_DIR}/stun.py
Restart=always
RestartSec=2
User=nobody
StandardOutput=append:/var/log/juntos-stun.log
StandardError=append:/var/log/juntos-stun.log

[Install]
WantedBy=multi-user.target
EOF

touch /var/log/juntos-stun.log
chown nobody /var/log/juntos-stun.log

systemctl daemon-reload
systemctl enable --now juntos-stun.service

echo "== Done =="
echo "Server public IP: $(curl -s ifconfig.me || echo 'unknown, run curl ifconfig.me manually')"
echo "Port: ${PORT}/udp"
echo "Logs:    journalctl -u juntos-stun -f   (or: tail -f /var/log/juntos-stun.log)"
echo "Status:  systemctl status juntos-stun"

#!/bin/bash

cp -f auto_static_ip.service /etc/systemd/system/auto_static_ip.service
cp -f auto_static_ip.py /usr/local/bin/auto_static_ip.py
chmod +x /usr/local/bin/auto_static_ip.py
systemctl enable auto_static_ip
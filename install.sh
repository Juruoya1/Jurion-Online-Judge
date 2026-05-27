#Copyright (c) 2026 Jurion. All Rights Reserved.
#SPDX-License-Identifier: MIT

#!/bin/bash

#Install Cern-httpd & Config
sudo apt install ./cern-httpd_3.0A-1_armhf.deb
sudo cern-httpd -v
sudo sed -i "s|./cgi-bin|$(pwd)/cgi-bin|" ./httpd.conf
sudo cp /etc/httpd.conf /etc/httpd.conf.bak
sudo cp ./httpd.conf /etc/httpd.conf

#Set 
sudo find ./ -type d -exec chmod 755 {} \;
sudo find ./ -type f -exec chmod 644 {} \;
find ./ -type f \( -name "*.cgi" -o -name "*.pl" -o -name "*.py" -o -name "*.sh" \) -exec chmod 755 {} \;

#Jurion Online Judge Starter
cd /mnt/hdd/jurionoj/html
# Start Server
sudo killall cern-httpd 2>/dev/null
sudo /usr/local/bin/cern-httpd -p 8080 &

#Finish
echo "JurionOJ started on port 8080"
echo "Http Link: http://$(hostname -I | awk '{print $1}'):8080"

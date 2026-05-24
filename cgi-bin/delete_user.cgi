#Copyright (c) 2026 Jurion. All Rights Reserved.
#SPDX-License-Identifier: MIT

#!/bin/sh
echo "Content-Type: text/html"
echo ""

token=$(echo "$QUERY_STRING" | sed 's/.*token=\([^&]*\).*/\1/')
target=$(echo "$QUERY_STRING" | sed 's/.*username=\([^&]*\).*/\1/')
user=""
role=0

if [ -f "/mnt/hdd/jurionoj/data/sessions/$token" ]; then
    user=$(cat "/mnt/hdd/jurionoj/data/sessions/$token")
    role=$(grep "^$user|" /mnt/hdd/jurionoj/data/users.txt | cut -d'|' -f3)
fi

if [ -z "$user" ] || [ "$role" -lt 1 ] || [ "$target" = "$user" ]; then
    echo "<html><body><h2>Permission denied</h2><a href='/admin.cgi?token=$token'>Back</a></body></html>"
    exit 0
fi

# 删除用户
grep -v "^$target|" /mnt/hdd/jurionoj/data/users.txt > /tmp/users.txt
mv /tmp/users.txt /mnt/hdd/jurionoj/data/users.txt

echo "<html><body><h2>User $target deleted</h2><a href='/admin.cgi?token=$token'>Back</a></body></html>"

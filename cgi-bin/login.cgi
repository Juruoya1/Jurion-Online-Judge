#Copyright (c) 2026 Jurion. All Rights Reserved.
#SPDX-License-Identifier: MIT

#!/bin/sh
echo "Content-Type: text/html"
echo ""

query="$QUERY_STRING"
username=$(echo "$query" | sed 's/.*username=\([^&]*\).*/\1/')
password=$(echo "$query" | sed 's/.*password=\([^&]*\).*/\1/')

if [ -z "$username" ] || [ -z "$password" ]; then
    echo "<html><body><h2>Login</h2>"
    echo "<form method='get'>"
    echo "Username: <input type='text' name='username'><br>"
    echo "Password: <input type='password' name='password'><br>"
    echo "<input type='submit' value='Login'></form></body></html>"
    exit 0
fi

if grep -q "^$username|$password|" /mnt/hdd/jurionoj/data/users.txt; then
    token=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | head -c 32)
    echo "$username" > "/mnt/hdd/jurionoj/data/sessions/$token"
    echo "<html><head>"
    echo "<meta http-equiv='refresh' content='0;url=/index.cgi?token=$token'>"
    echo "</head><body>Login success! Redirecting...</body></html>"
else
    echo "<html><body><h2>Login failed</h2><a href='/login.cgi'>Try again</a></body></html>"
fi

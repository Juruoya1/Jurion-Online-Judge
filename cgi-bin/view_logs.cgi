#Copyright (c) 2026 Jurion. All Rights Reserved.
#SPDX-License-Identifier: MIT

#!/bin/sh
echo "Content-Type: text/html"
echo ""

token=$(echo "$QUERY_STRING" | sed 's/.*token=\([^&]*\).*/\1/')
user=""
if [ -f "/mnt/hdd/jurionoj/data/sessions/$token" ]; then
    user=$(cat "/mnt/hdd/jurionoj/data/sessions/$token")
fi

role=$(grep "^$user|" /mnt/hdd/jurionoj/data/users.txt | cut -d'|' -f3)

if [ -z "$user" ] || [ "$role" != "2" ]; then
    echo "<html><body><h2>Permission denied. SU only.</h2><a href='/admin.cgi?token=$token'>Back</a></body></html>"
    exit 0
fi

logfile="/tmp/httpd.log"
lines=$(echo "$QUERY_STRING" | sed 's/.*lines=\([^&]*\).*/\1/')
if [ -z "$lines" ]; then
    lines=100
fi

full=$(echo "$QUERY_STRING" | grep "full=1")

echo "<html><head><meta charset='UTF-8'></head><body>"
echo "<h1>CERN httpd Log Viewer</h1>"
echo "<div><a href='/admin.cgi?token=$token'>Back to Admin</a> | <a href='/index.cgi?token=$token'>Home</a></div>"
echo "<hr>"

echo "<form method='get'>"
echo "<input type='hidden' name='token' value='$token'>"
echo "Lines: <input type='text' name='lines' value='$lines' size='5'>"
echo "<input type='submit' value='Refresh'>"
echo "<input type='submit' name='full' value='All' onclick=\"this.form.full.value='1'\">"
echo "</form>"

echo "<h2>"
if [ -n "$full" ]; then
    echo "Full log"
elif [ "$lines" = "all" ] || [ "$lines" = "0" ]; then
    echo "Full log"
else
    echo "Last $lines lines"
fi
echo "</h2>"

echo "<pre style='background:#f0f0f0; padding:10px; overflow:auto; font-size:11px; font-family:monospace;'>"
if [ -f "$logfile" ]; then
    if [ -n "$full" ] || [ "$lines" = "all" ] || [ "$lines" = "0" ]; then
        cat "$logfile"
    else
        tail -n "$lines" "$logfile"
    fi
else
    echo "Log file not found: $logfile"
fi
echo "</pre>"

echo "<p>Log file location: $logfile</p>"
echo "</body></html>"

#Copyright (c) 2026 Jurion. All Rights Reserved.
#SPDX-License-Identifier: MIT

#!/bin/sh
echo "Content-Type: text/html"
echo ""

token=$(echo "$QUERY_STRING" | sed 's/.*token=\([^&]*\).*/\1/')
echo "<!-- Debug: token=$token -->" >&2
if [ -n "$token" ] && [ -f "/mnt/hdd/jurionoj/data/sessions/$token" ]; then
    user=$(cat "/mnt/hdd/jurionoj/data/sessions/$token")
    echo "<div style='text-align:right; padding:10px; background:#f0f0f0;'>"
    echo "Welcome, <b>$user</b> | <a href='/logout.cgi?token=$token'>Logout</a>"
    echo "</div>"
else
    echo "<div style='text-align:right; padding:10px; background:#f0f0f0;'>"
    echo "<a href='/login.cgi'>Login</a> | <a href='/register.cgi'>Register</a>"
    echo "</div>"
fi

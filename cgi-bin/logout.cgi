#Copyright (c) 2026 Jurion. All Rights Reserved.
#SPDX-License-Identifier: MIT

#!/bin/sh
echo "Content-Type: text/html"
echo ""

token=$(echo "$QUERY_STRING" | sed 's/.*token=\([^&]*\).*/\1/')
if [ -n "$token" ]; then
    rm -f "/mnt/hdd/jurionoj/data/sessions/$token"
fi

echo "<html><head>"
echo "<meta http-equiv='refresh' content='0;url=/login.cgi'>"
echo "</head><body>"
echo "<p>Logged out. Redirecting to login page...</p>"
echo "</body></html>"

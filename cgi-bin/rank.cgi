#Copyright (c) 2026 Jurion. All Rights Reserved.
#SPDX-License-Identifier: MIT

#!/bin/sh
echo "Content-Type: text/html"
echo ""

# 解析参数
raw="$QUERY_STRING"
token=""
page="1"

# 提取 token
echo "$raw" | grep -q "token=" && token=$(echo "$raw" | sed 's/.*token=\([^&]*\).*/\1/')
# 提取 page
echo "$raw" | grep -q "page=" && page=$(echo "$raw" | sed 's/.*page=\([^&]*\).*/\1/')

[ -z "$page" ] && page=1
pagesize=20
start=$(( (page - 1) * pagesize + 1 ))
end=$(( page * pagesize ))

user=""
if [ -f "/mnt/hdd/jurionoj/data/sessions/$token" ]; then
    user=$(cat "/mnt/hdd/jurionoj/data/sessions/$token")
fi

echo "<html><head><meta charset='UTF-8'></head><body>"
echo "<div style='text-align:right; padding:10px; background:#f0f0f0;'>"
if [ -n "$user" ]; then
    echo "Welcome, <b>$user</b> | <a href='/logout.cgi?token=$token'>Logout</a> | <a href='/index.cgi?token=$token'>Home</a>"
else
    echo "<a href='/login.cgi'>Login</a> | <a href='/register.cgi'>Register</a>"
fi
echo "</div>"
echo "<h1>Ranking</h1>"
echo "<p>Ranked by number of Accepted solutions. Page $page</p>"
echo "<table border='1' cellpadding='5'>"
echo "<tr><th>Rank</th><th>Username</th><th>Nickname</th><th>Solved</th><th>Submissions</th><th>Rate</th></tr>"

tmpfile="/tmp/rank_$$.txt"
> "$tmpfile"

while IFS='|' read -r username password role uid nickname email; do
    [ -z "$username" ] && continue
    sub=$(grep -c "|$username|" /mnt/hdd/jurionoj/data/submissions.txt 2>/dev/null)
    ac=$(awk -F'|' -v u="$username" '$2==u {split($4,a,"/"); if(a[1]==a[2]) c++} END {print c+0}' /mnt/hdd/jurionoj/data/submissions.txt)
    echo "$ac|$sub|$username|$nickname" >> "$tmpfile"
done < /mnt/hdd/jurionoj/data/users.txt

sort -t'|' -k1 -rn "$tmpfile" > "${tmpfile}.sorted"
total=$(wc -l < "${tmpfile}.sorted")
total_pages=$(( (total + pagesize - 1) / pagesize ))

rank=1
while IFS='|' read -r ac sub name nick; do
    if [ "$rank" -ge "$start" ] && [ "$rank" -le "$end" ]; then
        [ "$ac" -eq 0 ] && [ "$sub" -eq 0 ] && continue
        [ "$sub" -eq 0 ] && continue
        rate=$((ac * 100 / sub))
        echo "<tr>"
        echo "<td>$rank</a></td>"
        echo "<td><a href='/profile.cgi?token=$token&user=$name'>$name</a></td>"
        echo "<td>$nick</a></td>"
        echo "<td>$ac</a></td>"
        echo "<td>$sub</a></td>"
        echo "<td>${rate}%</a></td>"
        echo "</tr>"
    fi
    rank=$((rank + 1))
done < "${tmpfile}.sorted"

rm -f "$tmpfile" "${tmpfile}.sorted"
echo "</table>"
echo "<p>"
[ "$page" -gt 1 ] && echo "<a href='/rank.cgi?token=$token&page=$((page-1))'>Prev</a> "
echo "Page $page / $total_pages "
[ "$page" -lt "$total_pages" ] && echo "<a href='/rank.cgi?token=$token&page=$((page+1))'>Next</a>"
echo "</p>"
echo "<p><a href='/index.cgi?token=$token'>Back to Home</a></p>"
echo "</body></html>"

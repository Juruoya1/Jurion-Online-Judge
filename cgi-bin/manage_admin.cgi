#!/bin/sh
echo "Content-Type: text/html"
echo ""

token=$(echo "$QUERY_STRING" | sed 's/.*token=\([^&]*\).*/\1/')
target_id=$(echo "$QUERY_STRING" | sed 's/.*userid=\([^&]*\).*/\1/')
action=$(echo "$QUERY_STRING" | sed 's/.*action=\([^&]*\).*/\1/')

user=""
if [ -f "/mnt/hdd/jurionoj/data/sessions/$token" ]; then
    user=$(cat "/mnt/hdd/jurionoj/data/sessions/$token")
fi

role=$(grep "^$user|" /mnt/hdd/jurionoj/data/users.txt | cut -d'|' -f3)

if [ -z "$user" ] || [ "$role" != "2" ] || [ -z "$target_id" ]; then
    echo "<html><body><h2>Permission denied. SU only.</h2><a href='/admin.cgi?token=$token'>Back</a></body></html>"
    exit 0
fi

# 通过 ID 查找用户名 - 修复：匹配 |ID| 格式
target_user=$(grep "|$target_id|" /mnt/hdd/jurionoj/data/users.txt | head -1 | cut -d'|' -f1)

if [ -z "$target_user" ]; then
    echo "<html><body><h2>User not found for ID: $target_id</h2><a href='/admin.cgi?token=$token'>Back</a></body></html>"
    exit 0
fi

if [ "$target_user" = "$user" ]; then
    echo "<html><body><h2>Cannot modify yourself</h2><a href='/admin.cgi?token=$token'>Back</a></body></html>"
    exit 0
fi

target_role=$(grep "^$target_user|" /mnt/hdd/jurionoj/data/users.txt | cut -d'|' -f3)

if [ "$action" = "add" ]; then
    if [ "$target_role" = "1" ] || [ "$target_role" = "2" ]; then
        echo "<html><body><h2>User $target_user is already admin or SU</h2>"
    else
        awk -F'|' -v u="$target_user" 'BEGIN{OFS="|"} $1==u {$3=1}1' /mnt/hdd/jurionoj/data/users.txt > /tmp/users.txt
        mv /tmp/users.txt /mnt/hdd/jurionoj/data/users.txt
        echo "<html><body><h2>User $target_user (ID: $target_id) is now admin</h2>"
    fi
elif [ "$action" = "remove" ]; then
    if [ "$target_role" != "1" ]; then
        echo "<html><body><h2>User $target_user is not an admin</h2>"
    else
        awk -F'|' -v u="$target_user" 'BEGIN{OFS="|"} $1==u {$3=0}1' /mnt/hdd/jurionoj/data/users.txt > /tmp/users.txt
        mv /tmp/users.txt /mnt/hdd/jurionoj/data/users.txt
        echo "<html><body><h2>User $target_user (ID: $target_id) is no longer admin</h2>"
    fi
else
    echo "<html><body><h2>Unknown action</h2>"
fi

echo "<a href='/admin.cgi?token=$token'>Back to Admin Panel</a></body></html>"

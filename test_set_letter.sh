#!/bin/bash -e
trap "kill 0" EXIT

PORT="$(ls /dev/cu.usbserial-*)"
PEER1="D8BC38FDD0BC"
PEER2="D8BC38F93930"
cat "$PORT" &
while true
do
    for ((i=30; i<=90; i++)); do
        echo -n "$i "
        stty -f $PORT 115200 & awk "BEGIN{printf \"$PEER1:%c\n\", $((i+1))}" > $PORT
        stty -f $PORT 115200 & awk "BEGIN{printf \"$PEER2:%c\n\", $i}" > $PORT
        sleep .1
    done
done
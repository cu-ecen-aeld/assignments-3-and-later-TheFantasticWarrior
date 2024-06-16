#!/bin/sh

case "$1" in
    start)
        start-stop-daemon -S -n aesdsocket /usr/bin/aesdsocket -- -d
        ;;
    stop)
        start-stop-daemon -K -n aesdsocket
        ;;
    *)
        echo "Invalid parameter"
    exit 1
esac

exit 0

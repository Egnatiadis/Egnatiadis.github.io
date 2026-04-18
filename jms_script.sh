#!/bin/bash

if [ $# -lt 4 ]; then
    exit 1
fi

while [[ $# -gt 0 ]]; do
    case $1 in
        -l)
            PATH_DIR="$2"
            shift 2
            ;;
        -c)
            COMMAND="$2"
            shift 2
            ;;
        *)
            N_VAL="$1"
            shift
            ;;
    esac
done

if [ ! -d "$PATH_DIR" ]; then
    exit 1
fi

case $COMMAND in
    list)
        ls -d $PATH_DIR/outputs_* 2>/dev/null
        ;;
    size)
        if [ -z "$N_VAL" ]; then
            du -sk $PATH_DIR/outputs_* 2>/dev/null | sort -n
        else
            du -sk $PATH_DIR/outputs_* 2>/dev/null | sort -n | tail -n "$N_VAL"
        fi
        ;;
    purge)
        rm -rf $PATH_DIR/outputs_*
        ;;
    *)
        exit 1
        ;;
esac
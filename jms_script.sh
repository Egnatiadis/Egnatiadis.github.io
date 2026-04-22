#!/bin/bash

# Έλεγχος ορισμάτων
if [ $# -lt 4 ]; then
    echo "Usage: $0 -l <path> -c <command> [n]"
    exit 1
fi

PATH_DIR=""
COMMAND=""
N_VAL=""

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
    echo "Error: Directory $PATH_DIR does not exist."
    exit 1
fi

case $COMMAND in
    list)
        ls -d "$PATH_DIR"/outputs_* 2>/dev/null
        ;;
    size*)
        # Αν το COMMAND είναι π.χ. "size 5", το διασπάμε
        CMD_PART=$(echo $COMMAND | cut -d' ' -f1)
        NUM_PART=$(echo $COMMAND | cut -d' ' -f2)
        
        # Αν δεν βρέθηκε νούμερο στο COMMAND, δες τη μεταβλητή N_VAL
        if [ "$CMD_PART" == "$NUM_PART" ]; then NUM_PART=$N_VAL; fi

        if [ -z "$NUM_PART" ]; then
            du -sk "$PATH_DIR"/outputs_* 2>/dev/null | sort -n
        else
            du -sk "$PATH_DIR"/outputs_* 2>/dev/null | sort -n | tail -n "$NUM_PART"
        fi
        ;;
    purge)
        # Διαγραφή καταλόγων
        rm -rf "$PATH_DIR"/outputs_*
        echo "Purge completed."
        ;;
    *)
        echo "Unknown command: $COMMAND"
        exit 1
        ;;
esac

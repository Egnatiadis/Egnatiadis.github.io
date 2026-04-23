LOG_DIR=""
COMMAND=""
N_VALUE=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -l) LOG_DIR="$2"; shift ;;
        -c) 
            COMMAND="$2"
            # Έλεγχος αν μετά το size υπάρχει αριθμός n
            if [[ "$COMMAND" == "size" && "$3" =~ ^[0-9]+$ ]]; then
                N_VALUE="$3"
                shift
            fi
            shift ;;
        *) shift ;;
    esac
done

# Έλεγχος αν δόθηκαν τα βασικά ορίσματα
if [[ -z "$LOG_DIR" || -z "$COMMAND" ]]; then
    echo "Usage: $0 -l <path> -c <command> [n]"
    exit 1
fi

if [ ! -d "$LOG_DIR" ]; then
    echo "Error: Directory $LOG_DIR does not exist."
    exit 1
fi

case $COMMAND in
    list)
        # Παρουσιάζει λίστα από τους καταλόγους των εργασιών
        echo "Job directories in $LOG_DIR:"
        ls -d "$LOG_DIR"/outputs_* 2>/dev/null
        ;;

    size)
        if [ -z "$N_VALUE" ]; then
            du -s "$LOG_DIR"/outputs_* 2>/dev/null | sort -n
        else
            du -s "$LOG_DIR"/outputs_* 2>/dev/null | sort -n | tail -n "$N_VALUE"
        fi
        ;;

    purge)
        echo "Deleting all job directories in $LOG_DIR..."
        rm -rf "$LOG_DIR"/outputs_*
        echo "Purge completed."
        ;;

    *)
        echo "Unknown command: $COMMAND"
        echo "Available commands: list, size [n], purge"
        exit 1
        ;;
esac

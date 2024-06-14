#!/bin/sh

if [ ! -d "$1" ] || [ -z "$2" ]; then
    exit 1
fi

# Initialize counters
num_files=0
num_lines=0

# Search for the pattern recursively in the specified directory
find "$1" -type f -exec grep -q "$2" {} \; -exec grep -H -c "$2" {} + | {
    while IFS=: read -r file_name count; do
        # Increment file count
        num_files=$((num_files + 1))
        # Increment line count
        num_lines=$((num_lines + count))
    done

    # Output results
    echo "The number of files are ${num_files} and the number of matching lines are ${num_lines}"
}

#!/bin/bash

cat <<'EOF'
Function|Supported|Notes
---|---|---
EOF

awk -F'\t' '{printf("%s|%s|%s\n", $1, $2, $3);}' <functions2.csv


cat <<EOF
EOF

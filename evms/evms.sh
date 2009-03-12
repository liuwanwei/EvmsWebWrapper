#!/bin/bash
#
# Use the EVMS CLI to list all volumes in the system.

evms -s << EOF 2>&1 
$@
EOF


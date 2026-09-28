#!/bin/bash

while make crash-test; do
    echo $SECONDS
done

echo "Failed"

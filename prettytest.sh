#!/bin/bash

FILES=`find programs/1-scan+parse/valid/ -type f \( -name "*.go" \)`
for FILE in $FILES
do
	echo "$FILE"
	./run.sh pretty $FILE >out1.go
	./run.sh pretty out1.go > out2.go
	cmp --silent out1.go out2.go || echo "Pretty error for $FILE"
done
rm out1.go
rm out2.go

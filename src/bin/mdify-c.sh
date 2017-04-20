#!/bin/bash
echo '```C' > $2
cat $1 >> $2
echo '```' >> $2


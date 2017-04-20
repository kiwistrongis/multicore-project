#!/bin/bash
echo '~~~rust' > $2
cat $1 >> $2
echo '~~~~~~~' >> $2


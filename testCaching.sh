#!/bin/bash
python3 -c "open('CacheTestFile','w').write('A'*1000000)"

for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do
    ./client "CacheTestFile" &
done
wait
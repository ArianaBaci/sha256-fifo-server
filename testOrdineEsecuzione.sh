#!/bin/bash

# Crea 5 file di dimensione diversa (in ordine INVERSO, così il test è più convincente)
echo "Creo i file di test..."
python3 -c "open('small.txt','w').write('A'*100)"
python3 -c "open('medium.txt','w').write('A'*1000)"
python3 -c "open('large.txt','w').write('A'*10000)"
python3 -c "open('xlarge.txt','w').write('A'*100000)"
python3 -c "open('xxlarge.txt','w').write('A'*1000000)"

# Lancia tutti i client quasi simultaneamente (senza sleep)
# così si accumulano in coda prima che il thread li processi
echo "Lancio i client..."
./client xxlarge.txt &
./client xlarge.txt &
./client large.txt &
./client medium.txt &
./client small.txt &

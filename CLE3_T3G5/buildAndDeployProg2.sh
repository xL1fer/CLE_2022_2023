echo "Compressing prog2."
rm -f  prog2.zip
zip -rq prog2.zip prog2.sh prog2
echo "Transfering prog2 data to remote node."
sshpass -f password ssh cle35@banana.ua.pt 'mkdir -p CLE3_T3G5'
sshpass -f password ssh cle35@banana.ua.pt 'rm -rf CLE3_T3G5/*'
sshpass -f password scp prog2.zip cle35@banana.ua.pt:CLE3_T3G5
echo "Decompressing data sent to the remote node."
sshpass -f password ssh cle35@banana.ua.pt 'cd CLE3_T3G5 ; unzip -uq prog2.zip'
echo "Compressing prog1."
rm -f  prog1.zip
zip -rq prog1.zip prog1.sh prog1
echo "Transfering prog1 data to remote node."
sshpass -f password ssh cle35@banana.ua.pt 'mkdir -p CLE3_T3G5'
sshpass -f password ssh cle35@banana.ua.pt 'rm -rf CLE3_T3G5/*'
sshpass -f password scp prog1.zip cle35@banana.ua.pt:CLE3_T3G5
echo "Decompressing data sent to the remote node."
sshpass -f password ssh cle35@banana.ua.pt 'cd CLE3_T3G5 ; unzip -uq prog1.zip'
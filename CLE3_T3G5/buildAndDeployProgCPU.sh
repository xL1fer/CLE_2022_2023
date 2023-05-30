echo "Compressing progcpu."
rm -f  progcpu.zip
zip -rq progcpu.zip progcpu.sh progcpu
echo "Transfering progcpu data to remote node."
sshpass -f password ssh cle35@banana.ua.pt 'mkdir -p CLE3_T3G5'
sshpass -f password ssh cle35@banana.ua.pt 'rm -rf CLE3_T3G5/*'
sshpass -f password scp progcpu.zip cle35@banana.ua.pt:CLE3_T3G5
echo "Decompressing data sent to the remote node."
sshpass -f password ssh cle35@banana.ua.pt 'cd CLE3_T3G5 ; unzip -uq progcpu.zip'
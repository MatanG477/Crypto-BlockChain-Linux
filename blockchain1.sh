#!/bin/bash
count=$1
rm -f info.txt blocks.txt main blockchain
wget api.blockcypher.com/v1/btc/main -o blockchain
GET "$(grep '"latest_url"' main | cut -d':' -f2- | tr -d ' ",')" >> blocks.txt 
for ((i=0; i<count; i++)); do 
	sed -n '2p;3p;5p;10p;12p;16p' blocks.txt >> info.txt 
	echo -e "\n\n\n" >> info.txt 
	GET "$(grep '"prev_block_url"' blocks.txt | cut -d':' -f2- | tr -d ' ",')" > blocks.txt
done

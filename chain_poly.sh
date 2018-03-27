#!/bin/bash
if [ "$#" -eq 2 ]; then
	if [ -x "$1" ]; then
		if [ -d "$2" ];then
			for f in "$2/"*; do
				line=$(head -n 1 "$f");
				if [ "$line" == START ] ; then
					fil="$f";
				fi
			done
			file=a;
			cp -f "$fil" "$file";
			sed  -i '/START/d' "$file";
			lastLine=START;
			until [ "$lastLine" == "STOP" ]; do
				lastLine=$(tail -n 1 "$file");
				if [ ! "$lastLine" == STOP ]; then
					file2=${lastLine#* };
				fi
				sed -i '$ d' "$file";
				cat "$file" >> out.txt;
				($1<out.txt) > outt.txt;
				cp -f outt.txt out.txt;
				> outt.txt;
				if [ ! "$lastLine" == "STOP" ]; then
					cp -f "$2/$file2" "$file";	
				fi
			done
			cat out.txt
			rm "$file";
			rm out.txt;
			rm outt.txt;
			exit 0;
		else echo "Wrong second argument";
			exit 1;
		fi
	else echo "Wrong first argument";
		exit 1;
	fi
else
	echo "Wrong number of parameters";
		exit 1;
fi

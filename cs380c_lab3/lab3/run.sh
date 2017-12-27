#!/usr/bin/env bash
THREE_ADDR_TO_C_TRANSLATOR=../../cs380c_lab1/lab1/run.sh

opt=0
for option in $*
do
	pos=`echo "$option" | awk 'match($0, "="){print RSTART}'`
	argument=${option:1:pos-2}
	value=${option:pos}
	if [ $argument = "backend" ]
	then
		if  [ $value = "c" ]
		then	
		    if [ $opt = 0 ]
		    then
			echo "enter~!"
			../../cs380c_lab1/lab1/run.sh
		    elif [ $opt = 11 ]
		    then
			./lab3 1 0 0 | ./ssa_to_3addr | ${THREE_ADDR_TO_C_TRANSLATOR}
		    elif [ $opt = 12 ]
		    then
			/lab3 2 0 0 | ./ssa_to_3addr | ${THREE_ADDR_TO_C_TRANSLATOR}
		    fi
		elif [ $value = "cfg" ]
		then
		    ./lab3 0 0 1
		elif [ $value = "ssa" ]
		then
		    if [ $opt = 0 ]
		    then
			./lab3
		    elif [ $opt = 11 ]
		    then
			./lab3 1 0 0 
		    elif [ $opt = 12 ]
		    then
			./lab3 2 0 0
		    fi

		elif [ $value = "3addr" ]
		then
	            if [ $opt = 0 ]
		    then
			echo 1
		    elif [ $opt = 11 ]
		    then
			./lab3 1 0 0 | ./ssa_to_3addr
		    elif [ $opt = 12 ]
		    then
			./lab3 2 0 0 | ./ssa_to_3addr
		    fi
		elif [ $value = "rep" ]
		then
		    if [ $opt = 0 ]
		    then
			echo 1
		    elif [ $opt = 11 ]
		    then
			./lab3 1 1 0
		    elif [ $opt = 12 ]
		    then
			./lab3 2 2 0
		    fi
		fi
	elif [ $argument = "opt" ]
	then
		if [ $value = "ssa,scp" ]
		then
			opt=11
		elif [ $value = "ssa,licm" ]
		then
			opt=12
		fi		
	fi
done
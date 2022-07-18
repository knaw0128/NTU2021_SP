#!/bin/bash
para_num=$#
if [ ${para_num} -lt 6 ]; then
    echo -e "parameter number error " && exit 0
fi

declare -i host_num=0
declare -i player_num=0
declare -i lucky_num=0

while [ "$#" -gt 0 ]; do    
    if [ "${1}" == "-m" ]; then
        host_num=${2}
    elif [ "${1}" == "-n" ]; then
        player_num=$2
    elif [ "${1}" == "-l" ]; then
        lucky_num=$2
    else
        echo -e "wrong parameter"
        exit 0
    fi
    shift 2
done

for (( i=0; $i <= $host_num; i++))
do
    mkfifo "./fifo_${i}.tmp" 
done

n=$player_num
declare -A arr
declare -a scoreboard

declare -i idx=0

for (( a=1; a <= n-7; a++)) do
    for (( b=a+1; b <= n-6; b++)) do
        for (( c=b+1; c <= n-5; c++)) do
            for (( d=c+1; d <= n-4; d++)) do
                for (( e=d+1; e <= n-3; e++)) do
                    for (( f=e+1; f <= n-2; f++)) do
                        for (( g=f+1; g <= n-1; g++)) do
                            for (( h=g+1; h <= n; h++)) do
                                declare -i now=0
                                arr[$idx,$now]=$a
                                now=now+1
                                arr[$idx,$now]=$b
                                now=now+1
                                arr[$idx,$now]=$c
                                now=now+1
                                arr[$idx,$now]=$d
                                now=now+1
                                arr[$idx,$now]=$e
                                now=now+1
                                arr[$idx,$now]=$f
                                now=now+1
                                arr[$idx,$now]=$g
                                now=now+1
                                arr[$idx,$now]=$h
                                idx=idx+1
                            done
                        done
                    done
                done
            done
        done
    done
done

for (( i=1; i <= player_num; i++)) do
    scoreboard[$i]=0
done

(( host_num <= idx )) && first_do=$host_num || first_do=$idx

for (( i=0; i<first_do; i++)) do
    declare -i tar
    ((tar=i+1))
    ./host -m $tar -d 0 -l $lucky_num &
    for ((j=0; j<7; j++)) do
        echo -n "${arr[$i,$j]} " >> "fifo_$tar.tmp" &
    done
    echo "${arr[$i,7]}\n" >> "fifo_$tar.tmp" &
done

echo -e "assign end"
echo "first do = $first_do    host_num = $host_num mission = $idx"

# if [ $player_num -eq 1 ]; then
#     echo  "\n" >> "fifo_0.tmp" &
# fi

for (( i=first_do; i<idx; i++)) do
    
    read done_host
    for ((j=0; j<8; j++)) do
        read player
        players=(${player})
        ((scoreboard[${players[0]}]=scoreboard[${players[0]}]+${players[1]}))
    done < "./fifo_0.tmp"

    for ((j=0; j<7; j++)) do
        echo -n "${arr[$i,$j]} " >> "fifo_$done_host.tmp" &
    done
    echo "${arr[$i,7]}\n" >> "fifo_$done_host.tmp" &
    
done < "./fifo_0.tmp"


echo -e "second assign end"
((ne=idx%host_num))
echo "left = $ne"

for (( i=1; i<=(idx%host_num); i++)) do

    read done_host
    echo -e $done_host
    for ((j=0; j<8; j++)) do
        read player
        echo -e $player
        players=(${player})
        echo -e "${players[0]}  ${players[1]}"
        ((scoreboard[${players[0]}]=scoreboard[${players[0]}]+${players[1]}))
    done
    
done < "./fifo_0.tmp"

echo -e "get end"

for (( i=1; i<=host_num; i++)) do
    echo "-1 -1 -1 -1 -1 -1 -1 -1\n" >> "fifo_$i.tmp" &
done

for (( i=1; i<=player_num; i++)) do
    echo "player $i : ${scoreboard[$i]}\n"
done

echo -e "terminate end"


rm *.tmp




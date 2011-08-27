for file in $1*.ppm
do
    for t in 0 45 90 135 180 225 270 315
    do
        for a in 5 10 15 20 30
        do
	        echo "Calling $file with alpha $a tetha $t"
    	    ./../../bin/linux/release/C2G_GPU $file $a $t
        done
    done
done  

for file in $1*.ppm
do
    convert $file /home/wendell/workspaceCILAMCE/C2g_CPUv2/data/img/$file \
    \( -clone 0 -clone 1 -compose difference -composite -threshold 0 \) \
    -delete 1 -alpha off -compose copy_opacity -composite -trim \
    $1dif/diff$file.ppm
    
done

set xlabel "Multiread Size (#)"
set ylabel "Per Object Latency (us)"
set logscale x
set key autotitle columnhead
set terminal postscript eps color size 5,10
set yrange [0.28:2.7]

set title "Per Object Latency vs. Multiread Size"
set output "multiread.eps"
plot "multiread.2.csv" u 3:($2==1&&$5==32?$11:1/0) with lines lc 1 title "Servers=1, ValueSize=32B", \
    "multiread.2.csv" u 3:($2==3&&$5==32?$11:1/0) with lines lc 3 title "Servers=3, ValueSize=32B", \
    "multiread.2.csv" u 3:($2==5&&$5==32?$11:1/0) with lines lc 5 title "Servers=5, ValueSize=32B", \
    "multiread.2.csv" u 3:($2==7&&$5==32?$11:1/0) with lines lc 7 title "Servers=7, ValueSize=32B", \
    "multiread.2.csv" u 3:($2==1&&$5==64?$11:1/0) with lines lc 1 title "Servers=1, ValueSize=64B", \
    "multiread.2.csv" u 3:($2==3&&$5==64?$11:1/0) with lines lc 3 title "Servers=3, ValueSize=64B", \
    "multiread.2.csv" u 3:($2==5&&$5==64?$11:1/0) with lines lc 5 title "Servers=5, ValueSize=64B", \
    "multiread.2.csv" u 3:($2==7&&$5==64?$11:1/0) with lines lc 7 title "Servers=7, ValueSize=64B", \
    "multiread.2.csv" u 3:($2==1&&$5==128?$11:1/0) with lines lc 1 title "Servers=1, ValueSize=128B", \
    "multiread.2.csv" u 3:($2==3&&$5==128?$11:1/0) with lines lc 3 title "Servers=3, ValueSize=128B", \
    "multiread.2.csv" u 3:($2==5&&$5==128?$11:1/0) with lines lc 5 title "Servers=5, ValueSize=128B", \
    "multiread.2.csv" u 3:($2==7&&$5==128?$11:1/0) with lines lc 7 title "Servers=7, ValueSize=128B", \
    "multiread.2.csv" u 3:($2==1&&$5==256?$11:1/0) with lines lc 1 title "Servers=1, ValueSize=256B", \
    "multiread.2.csv" u 3:($2==3&&$5==256?$11:1/0) with lines lc 3 title "Servers=3, ValueSize=256B", \
    "multiread.2.csv" u 3:($2==5&&$5==256?$11:1/0) with lines lc 5 title "Servers=5, ValueSize=256B", \
    "multiread.2.csv" u 3:($2==7&&$5==256?$11:1/0) with lines lc 7 title "Servers=7, ValueSize=256B", \
    "multiread.2.csv" u 3:($2==1&&$5==512?$11:1/0) with lines lc 1 title "Servers=1, ValueSize=512B", \
    "multiread.2.csv" u 3:($2==3&&$5==512?$11:1/0) with lines lc 3 title "Servers=3, ValueSize=512B", \
    "multiread.2.csv" u 3:($2==5&&$5==512?$11:1/0) with lines lc 5 title "Servers=5, ValueSize=512B", \
    "multiread.2.csv" u 3:($2==7&&$5==512?$11:1/0) with lines lc 7 title "Servers=7, ValueSize=512B", \
    "multiread.2.csv" u 3:($2==1&&$5==1024?$11:1/0) with lines lc 1 title "Servers=1, ValueSize=1024B", \
    "multiread.2.csv" u 3:($2==3&&$5==1024?$11:1/0) with lines lc 3 title "Servers=3, ValueSize=1024B", \
    "multiread.2.csv" u 3:($2==5&&$5==1024?$11:1/0) with lines lc 5 title "Servers=5, ValueSize=1024B", \
    "multiread.2.csv" u 3:($2==7&&$5==1024?$11:1/0) with lines lc 7 title "Servers=7, ValueSize=1024B", \
    "multiread.2.csv" u 3:($2==1&&$5==2048?$11:1/0) with lines lc 1 title "Servers=1, ValueSize=2048B", \
    "multiread.2.csv" u 3:($2==3&&$5==2048?$11:1/0) with lines lc 3 title "Servers=3, ValueSize=2048B", \
    "multiread.2.csv" u 3:($2==5&&$5==2048?$11:1/0) with lines lc 5 title "Servers=5, ValueSize=2048B", \
    "multiread.2.csv" u 3:($2==7&&$5==2048?$11:1/0) with lines lc 7 title "Servers=7, ValueSize=2048B", \

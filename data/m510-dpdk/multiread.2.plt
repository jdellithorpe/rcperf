set xlabel "Multiread Size (#)"
set ylabel "Per Object Latency (us)"
set logscale x
set key autotitle columnhead
set terminal postscript eps color size 5,10
set yrange [0.28:2.7]

set title "Per Object Latency vs. Multiread Size"
set output "multiread.vs_32.eps"
plot "multiread.csv" u 3:($2==1&&$5==32?$11:1/0) with lines title "Servers=1, ValueSize=32B", \
    "multiread.csv" u 3:($2==2&&$5==32?$11:1/0) with lines title "Servers=2, ValueSize=32B", \
    "multiread.csv" u 3:($2==3&&$5==32?$11:1/0) with lines title "Servers=3, ValueSize=32B", \
    "multiread.csv" u 3:($2==4&&$5==32?$11:1/0) with lines title "Servers=4, ValueSize=32B", \
    "multiread.csv" u 3:($2==5&&$5==32?$11:1/0) with lines title "Servers=5, ValueSize=32B", \
    "multiread.csv" u 3:($2==6&&$5==32?$11:1/0) with lines title "Servers=6, ValueSize=32B", \
    "multiread.csv" u 3:($2==7&&$5==32?$11:1/0) with lines title "Servers=7, ValueSize=32B", \
    "multiread.csv" u 3:($2==8&&$5==32?$11:1/0) with lines title "Servers=8, ValueSize=32B"

set title "Per Object Latency vs. Multiread Size"
set output "multiread.vs_64.eps"
plot "multiread.csv" u 3:($2==1&&$5==64?$11:1/0) with lines title "Servers=1, ValueSize=64B", \
    "multiread.csv" u 3:($2==2&&$5==64?$11:1/0) with lines title "Servers=2, ValueSize=64B", \
    "multiread.csv" u 3:($2==3&&$5==64?$11:1/0) with lines title "Servers=3, ValueSize=64B", \
    "multiread.csv" u 3:($2==4&&$5==64?$11:1/0) with lines title "Servers=4, ValueSize=64B", \
    "multiread.csv" u 3:($2==5&&$5==64?$11:1/0) with lines title "Servers=5, ValueSize=64B", \
    "multiread.csv" u 3:($2==6&&$5==64?$11:1/0) with lines title "Servers=6, ValueSize=64B", \
    "multiread.csv" u 3:($2==7&&$5==64?$11:1/0) with lines title "Servers=7, ValueSize=64B", \
    "multiread.csv" u 3:($2==8&&$5==64?$11:1/0) with lines title "Servers=8, ValueSize=64B"

set title "Per Object Latency vs. Multiread Size"
set output "multiread.vs_128.eps"
plot "multiread.csv" u 3:($2==1&&$5==128?$11:1/0) with lines title "Servers=1, ValueSize=128B", \
    "multiread.csv" u 3:($2==2&&$5==128?$11:1/0) with lines title "Servers=2, ValueSize=128B", \
    "multiread.csv" u 3:($2==3&&$5==128?$11:1/0) with lines title "Servers=3, ValueSize=128B", \
    "multiread.csv" u 3:($2==4&&$5==128?$11:1/0) with lines title "Servers=4, ValueSize=128B", \
    "multiread.csv" u 3:($2==5&&$5==128?$11:1/0) with lines title "Servers=5, ValueSize=128B", \
    "multiread.csv" u 3:($2==6&&$5==128?$11:1/0) with lines title "Servers=6, ValueSize=128B", \
    "multiread.csv" u 3:($2==7&&$5==128?$11:1/0) with lines title "Servers=7, ValueSize=128B", \
    "multiread.csv" u 3:($2==8&&$5==128?$11:1/0) with lines title "Servers=8, ValueSize=128B"

set title "Per Object Latency vs. Multiread Size"
set output "multiread.vs_256.eps"
plot "multiread.csv" u 3:($2==1&&$5==256?$11:1/0) with lines title "Servers=1, ValueSize=256B", \
    "multiread.csv" u 3:($2==2&&$5==256?$11:1/0) with lines title "Servers=2, ValueSize=256B", \
    "multiread.csv" u 3:($2==3&&$5==256?$11:1/0) with lines title "Servers=3, ValueSize=256B", \
    "multiread.csv" u 3:($2==4&&$5==256?$11:1/0) with lines title "Servers=4, ValueSize=256B", \
    "multiread.csv" u 3:($2==5&&$5==256?$11:1/0) with lines title "Servers=5, ValueSize=256B", \
    "multiread.csv" u 3:($2==6&&$5==256?$11:1/0) with lines title "Servers=6, ValueSize=256B", \
    "multiread.csv" u 3:($2==7&&$5==256?$11:1/0) with lines title "Servers=7, ValueSize=256B", \
    "multiread.csv" u 3:($2==8&&$5==256?$11:1/0) with lines title "Servers=8, ValueSize=256B"

set title "Per Object Latency vs. Multiread Size"
set output "multiread.vs_512.eps"
plot "multiread.csv" u 3:($2==1&&$5==512?$11:1/0) with lines title "Servers=1, ValueSize=512B", \
    "multiread.csv" u 3:($2==2&&$5==512?$11:1/0) with lines title "Servers=2, ValueSize=512B", \
    "multiread.csv" u 3:($2==3&&$5==512?$11:1/0) with lines title "Servers=3, ValueSize=512B", \
    "multiread.csv" u 3:($2==4&&$5==512?$11:1/0) with lines title "Servers=4, ValueSize=512B", \
    "multiread.csv" u 3:($2==5&&$5==512?$11:1/0) with lines title "Servers=5, ValueSize=512B", \
    "multiread.csv" u 3:($2==6&&$5==512?$11:1/0) with lines title "Servers=6, ValueSize=512B", \
    "multiread.csv" u 3:($2==7&&$5==512?$11:1/0) with lines title "Servers=7, ValueSize=512B", \
    "multiread.csv" u 3:($2==8&&$5==512?$11:1/0) with lines title "Servers=8, ValueSize=512B"

set title "Per Object Latency vs. Multiread Size"
set output "multiread.vs_1024.eps"
plot "multiread.csv" u 3:($2==1&&$5==1024?$11:1/0) with lines title "Servers=1, ValueSize=1024B", \
    "multiread.csv" u 3:($2==2&&$5==1024?$11:1/0) with lines title "Servers=2, ValueSize=1024B", \
    "multiread.csv" u 3:($2==3&&$5==1024?$11:1/0) with lines title "Servers=3, ValueSize=1024B", \
    "multiread.csv" u 3:($2==4&&$5==1024?$11:1/0) with lines title "Servers=4, ValueSize=1024B", \
    "multiread.csv" u 3:($2==5&&$5==1024?$11:1/0) with lines title "Servers=5, ValueSize=1024B", \
    "multiread.csv" u 3:($2==6&&$5==1024?$11:1/0) with lines title "Servers=6, ValueSize=1024B", \
    "multiread.csv" u 3:($2==7&&$5==1024?$11:1/0) with lines title "Servers=7, ValueSize=1024B", \
    "multiread.csv" u 3:($2==8&&$5==1024?$11:1/0) with lines title "Servers=8, ValueSize=1024B"

set title "Per Object Latency vs. Multiread Size"
set output "multiread.vs_2048.eps"
plot "multiread.csv" u 3:($2==1&&$5==2048?$11:1/0) with lines title "Servers=1, ValueSize=2048B", \
    "multiread.csv" u 3:($2==2&&$5==2048?$11:1/0) with lines title "Servers=2, ValueSize=2048B", \
    "multiread.csv" u 3:($2==3&&$5==2048?$11:1/0) with lines title "Servers=3, ValueSize=2048B", \
    "multiread.csv" u 3:($2==4&&$5==2048?$11:1/0) with lines title "Servers=4, ValueSize=2048B", \
    "multiread.csv" u 3:($2==5&&$5==2048?$11:1/0) with lines title "Servers=5, ValueSize=2048B", \
    "multiread.csv" u 3:($2==6&&$5==2048?$11:1/0) with lines title "Servers=6, ValueSize=2048B", \
    "multiread.csv" u 3:($2==7&&$5==2048?$11:1/0) with lines title "Servers=7, ValueSize=2048B", \
    "multiread.csv" u 3:($2==8&&$5==2048?$11:1/0) with lines title "Servers=8, ValueSize=2048B"

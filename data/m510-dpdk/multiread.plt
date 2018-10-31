set title "Per Object Latency vs. Multiread Size"
set xlabel "Multiread Size (#)"
set ylabel "Per Object Latency (us)"
set logscale x
set key autotitle columnhead
set terminal postscript eps color
set output "multiread.eps"
plot "multiread.csv" u 3:($2==1?$11:1/0) with lines title "Servers=1", \
    "multiread.csv" u 3:($2==2?$11:1/0) with lines title "Servers=2", \
    "multiread.csv" u 3:($2==3?$11:1/0) with lines title "Servers=3", \
    "multiread.csv" u 3:($2==4?$11:1/0) with lines title "Servers=4", \
    "multiread.csv" u 3:($2==5?$11:1/0) with lines title "Servers=5", \
    "multiread.csv" u 3:($2==6?$11:1/0) with lines title "Servers=6", \
    "multiread.csv" u 3:($2==7?$11:1/0) with lines title "Servers=7", \
    "multiread.csv" u 3:($2==8?$11:1/0) with lines title "Servers=8" 

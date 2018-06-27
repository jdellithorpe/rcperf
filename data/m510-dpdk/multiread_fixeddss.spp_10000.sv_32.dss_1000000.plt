set title "Latency vs. Multiread Size"
set xlabel "Objects in Multiread"
set ylabel "Latency (us)"
set key autotitle columnhead
set terminal postscript eps color
set output "multiread_fixeddss.spp_10000.sv_32.dss_1000000.eps"
plot for [i=2:12] "multiread_fixeddss.spp_10000.sv_32.dss_1000000.csv" u 1:i with lines lc i

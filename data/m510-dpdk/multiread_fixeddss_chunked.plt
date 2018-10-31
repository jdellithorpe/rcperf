set title "Total Latency of Reading 1M 100B Objects vs. Multiread Size"
set xlabel "Multiread Size (#)"
set ylabel "Total Latency (us)"
set logscale x
set key autotitle columnhead
set terminal postscript eps color
set output "multiread_fixeddss_chunked.eps"
plot "multiread_fixeddss_chunked.csv" u 4:12 with lines

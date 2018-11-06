set xlabel "Multiread Size (#)"
set ylabel "Total Latency (us)"
set logscale x
set key autotitle columnhead
set terminal postscript eps color
set title "Total Latency of Reading 1M 100B Objects vs. Multiread Size"
set output "multiread_fixeddss_chunked.vs_100.eps"
plot "multiread_fixeddss_chunked.csv" u 4:($3==1&&$6==100?$12:1/0) with lines title "Servers=1, ValueSize=100B", \
    "multiread_fixeddss_chunked.csv" u 4:($3==2&&$6==100?$12:1/0) with lines title "Servers=2, ValueSize=100B", \
    "multiread_fixeddss_chunked.csv" u 4:($3==4&&$6==100?$12:1/0) with lines title "Servers=4, ValueSize=100B", \
    "multiread_fixeddss_chunked.csv" u 4:($3==8&&$6==100?$12:1/0) with lines title "Servers=8, ValueSize=100B"
set title "Total Latency of Reading 1M 1KB Objects vs. Multiread Size"
set output "multiread_fixeddss_chunked.vs_1000.eps"
plot "multiread_fixeddss_chunked.csv" u 4:($3==1&&$6==1000?$12:1/0) with lines title "Servers=1, ValueSize=1KB", \
    "multiread_fixeddss_chunked.csv" u 4:($3==2&&$6==1000?$12:1/0) with lines title "Servers=2, ValueSize=1KB", \
    "multiread_fixeddss_chunked.csv" u 4:($3==4&&$6==1000?$12:1/0) with lines title "Servers=4, ValueSize=1KB", \
    "multiread_fixeddss_chunked.csv" u 4:($3==8&&$6==1000?$12:1/0) with lines title "Servers=8, ValueSize=1KB"

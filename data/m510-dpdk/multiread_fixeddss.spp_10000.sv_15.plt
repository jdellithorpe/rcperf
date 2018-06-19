set title "Latency vs. Multiread Size"
set xlabel "Multiread Size (Objects)"
set ylabel "Latency (us)"
set logscale xy
set key autotitle columnhead
set terminal postscript eps color
set output "multiread_fixeddss.spp_10000.sv_15.eps"
plot for [i=2:12] "multiread_fixeddss.spp_10000.sv_15.dss_100.csv" u 1:i with lines lc i
plot for [i=2:12] "multiread_fixeddss.spp_10000.sv_15.dss_268.csv" u 1:i with lines lc i
plot for [i=2:12] "multiread_fixeddss.spp_10000.sv_15.dss_718.csv" u 1:i with lines lc i
plot for [i=2:12] "multiread_fixeddss.spp_10000.sv_15.dss_1926.csv" u 1:i with lines lc i
plot for [i=2:12] "multiread_fixeddss.spp_10000.sv_15.dss_5166.csv" u 1:i with lines lc i
plot for [i=2:12] "multiread_fixeddss.spp_10000.sv_15.dss_13858.csv" u 1:i with lines lc i
plot for [i=2:12] "multiread_fixeddss.spp_10000.sv_15.dss_37176.csv" u 1:i with lines lc i
plot for [i=2:12] "multiread_fixeddss.spp_10000.sv_15.dss_99731.csv" u 1:i with lines lc i

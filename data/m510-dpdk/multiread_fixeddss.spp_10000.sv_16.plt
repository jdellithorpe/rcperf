set title "Latency vs. Multiread Size"
set xlabel "# Objects in Multiread"
set ylabel "Latency (us)"
set logscale y
set key autotitle columnhead
set terminal postscript eps color
set output "multiread_fixeddss.spp_10000.sv_16.eps"
plot "multiread_fixeddss.spp_10000.sv_16.dss_10000.csv" u 1:7 with lines lc 1 title "10KB", \
    "multiread_fixeddss.spp_10000.sv_16.dss_31622.csv" u 1:7 with lines lc 2 title "32KB", \
    "multiread_fixeddss.spp_10000.sv_16.dss_99997.csv" u 1:7 with lines lc 3 title "100KB", \
    "multiread_fixeddss.spp_10000.sv_16.dss_316218.csv" u 1:7 with lines lc 4 title "316KB", \
    "multiread_fixeddss.spp_10000.sv_16.dss_999969.csv" u 1:7 with lines lc 5 title "1MB"


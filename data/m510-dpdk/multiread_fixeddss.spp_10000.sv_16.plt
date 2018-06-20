set title "Latency vs. Multiread Size"
set xlabel "# Objects in Multiread"
set ylabel "Latency (us)"
set logscale xy
set key autotitle columnhead
set terminal postscript eps color
set output "multiread_fixeddss.spp_10000.sv_16.eps"
plot "multiread_fixeddss.spp_10000.sv_16.dss_10000.csv" u 1:7 with lines lc 1, \
    "multiread_fixeddss.spp_10000.sv_16.dss_31622.csv" u 1:7 with lines lc 2, \
    "multiread_fixeddss.spp_10000.sv_16.dss_99997.csv" u 1:7 with lines lc 3, \
    "multiread_fixeddss.spp_10000.sv_16.dss_316218.csv" u 1:7 with lines lc 4, \
    "multiread_fixeddss.spp_10000.sv_16.dss_999969.csv" u 1:7 with lines lc 5


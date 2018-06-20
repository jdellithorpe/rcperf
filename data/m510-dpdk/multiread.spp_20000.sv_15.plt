set title "Latency vs. Multiread Size"
set xlabel "Object Size (B)"
set ylabel "Latency (us)"
set logscale xy
set key autotitle columnhead
set terminal postscript eps color
set output "multiread.spp_20000.sv_15.eps"
plot "multiread.spp_20000.sv_15.mrs_1.ks_30.csv" u 1:7 with lines lc 1 title "MRS=1", \
    "multiread.spp_20000.sv_15.mrs_2.ks_30.csv" u 1:7 with lines lc 2 title "MRS=2", \
    "multiread.spp_20000.sv_15.mrs_3.ks_30.csv" u 1:7 with lines lc 3 title "MRS=3", \
    "multiread.spp_20000.sv_15.mrs_4.ks_30.csv" u 1:7 with lines lc 4 title "MRS=4", \
    "multiread.spp_20000.sv_15.mrs_5.ks_30.csv" u 1:7 with lines lc 5 title "MRS=5", \
    "multiread.spp_20000.sv_15.mrs_6.ks_30.csv" u 1:7 with lines lc 6 title "MRS=6", \
    "multiread.spp_20000.sv_15.mrs_7.ks_30.csv" u 1:7 with lines lc 7 title "MRS=7", \
    "multiread.spp_20000.sv_15.mrs_8.ks_30.csv" u 1:7 with lines lc 8 title "MRS=8", \
    "multiread.spp_20000.sv_15.mrs_9.ks_30.csv" u 1:7 with lines lc 9 title "MRS=9", \
    "multiread.spp_20000.sv_15.mrs_10.ks_30.csv" u 1:7 with lines lc 10 title "MRS=10", \
    "multiread.spp_20000.sv_15.mrs_11.ks_30.csv" u 1:7 with lines lc 11 title "MRS=11", \
    "multiread.spp_20000.sv_15.mrs_12.ks_30.csv" u 1:7 with lines lc 12 title "MRS=12", \
    "multiread.spp_20000.sv_15.mrs_13.ks_30.csv" u 1:7 with lines lc 13 title "MRS=13", \
    "multiread.spp_20000.sv_15.mrs_14.ks_30.csv" u 1:7 with lines lc 14 title "MRS=14", \
    "multiread.spp_20000.sv_15.mrs_15.ks_30.csv" u 1:7 with lines lc 15 title "MRS=15", \
    "multiread.spp_20000.sv_15.mrs_16.ks_30.csv" u 1:7 with lines lc 16 title "MRS=16"

set title "Latency vs. Multiread Size"
set xlabel "Object Size (B)"
set ylabel "Latency (us)"
set logscale xy
set key autotitle columnhead
set terminal postscript eps color
set output "multiread.spp_10000.sv_32.eps"
plot "multiread.spp_10000.sv_32.mrs_16.ks_30.csv" u 1:7 with lines lc 1 title "MRS=16", \
    "multiread.spp_10000.sv_32.mrs_17.ks_30.csv" u 1:7 with lines lc 3 title "MRS=17", \
    "multiread.spp_10000.sv_32.mrs_18.ks_30.csv" u 1:7 with lines lc 4 title "MRS=18", \
    "multiread.spp_10000.sv_32.mrs_19.ks_30.csv" u 1:7 with lines lc 5 title "MRS=19", \
    "multiread.spp_10000.sv_32.mrs_20.ks_30.csv" u 1:7 with lines lc 6 title "MRS=20", \
    "multiread.spp_10000.sv_32.mrs_21.ks_30.csv" u 1:7 with lines lc 7 title "MRS=21", \
    "multiread.spp_10000.sv_32.mrs_22.ks_30.csv" u 1:7 with lines lc 8 title "MRS=22", \
    "multiread.spp_10000.sv_32.mrs_23.ks_30.csv" u 1:7 with lines lc 9 title "MRS=23", \
    "multiread.spp_10000.sv_32.mrs_24.ks_30.csv" u 1:7 with lines lc 10 title "MRS=24", \
    "multiread.spp_10000.sv_32.mrs_25.ks_30.csv" u 1:7 with lines lc 11 title "MRS=25", \
    "multiread.spp_10000.sv_32.mrs_26.ks_30.csv" u 1:7 with lines lc 12 title "MRS=26", \
    "multiread.spp_10000.sv_32.mrs_27.ks_30.csv" u 1:7 with lines lc 13 title "MRS=27", \
    "multiread.spp_10000.sv_32.mrs_28.ks_30.csv" u 1:7 with lines lc 14 title "MRS=28", \
    "multiread.spp_10000.sv_32.mrs_29.ks_30.csv" u 1:7 with lines lc 15 title "MRS=29", \
    "multiread.spp_10000.sv_32.mrs_30.ks_30.csv" u 1:7 with lines lc 15 title "MRS=30", \
    "multiread.spp_10000.sv_32.mrs_31.ks_30.csv" u 1:7 with lines lc 15 title "MRS=31", \
    "multiread.spp_10000.sv_32.mrs_32.ks_30.csv" u 1:7 with lines lc 15 title "MRS=32"

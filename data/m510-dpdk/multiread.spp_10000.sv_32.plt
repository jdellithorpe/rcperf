set title "Latency vs. Multiread Size"
set xlabel "Object Size (B)"
set ylabel "Latency (us)"
set logscale xy
set key autotitle columnhead
set terminal postscript eps color
set output "multiread.spp_10000.sv_32.eps"
plot "multiread.spp_10000.sv_32.mrs_1.ks_30.csv" u 1:7 with lines lc 1 title "MRS=1", \
    "multiread.spp_10000.sv_32.mrs_2.ks_30.csv" u 1:7 with lines lc 2 title "MRS=2", \
    "multiread.spp_10000.sv_32.mrs_3.ks_30.csv" u 1:7 with lines lc 3 title "MRS=3", \
    "multiread.spp_10000.sv_32.mrs_4.ks_30.csv" u 1:7 with lines lc 4 title "MRS=4", \
    "multiread.spp_10000.sv_32.mrs_5.ks_30.csv" u 1:7 with lines lc 5 title "MRS=5", \
    "multiread.spp_10000.sv_32.mrs_6.ks_30.csv" u 1:7 with lines lc 6 title "MRS=6", \
    "multiread.spp_10000.sv_32.mrs_7.ks_30.csv" u 1:7 with lines lc 7 title "MRS=7", \
    "multiread.spp_10000.sv_32.mrs_8.ks_30.csv" u 1:7 with lines lc 8 title "MRS=8", \
    "multiread.spp_10000.sv_32.mrs_9.ks_30.csv" u 1:7 with lines lc 9 title "MRS=9", \
    "multiread.spp_10000.sv_32.mrs_10.ks_30.csv" u 1:7 with lines lc 10 title "MRS=10", \
    "multiread.spp_10000.sv_32.mrs_11.ks_30.csv" u 1:7 with lines lc 11 title "MRS=11", \
    "multiread.spp_10000.sv_32.mrs_12.ks_30.csv" u 1:7 with lines lc 12 title "MRS=12", \
    "multiread.spp_10000.sv_32.mrs_13.ks_30.csv" u 1:7 with lines lc 13 title "MRS=13", \
    "multiread.spp_10000.sv_32.mrs_14.ks_30.csv" u 1:7 with lines lc 14 title "MRS=14", \
    "multiread.spp_10000.sv_32.mrs_15.ks_30.csv" u 1:7 with lines lc 15 title "MRS=15", \
    "multiread.spp_10000.sv_32.mrs_16.ks_30.csv" u 1:7 with lines lc 16 title "MRS=16", \
    "multiread.spp_10000.sv_32.mrs_17.ks_30.csv" u 1:7 with lines lc 17 title "MRS=17", \
    "multiread.spp_10000.sv_32.mrs_18.ks_30.csv" u 1:7 with lines lc 18 title "MRS=18", \
    "multiread.spp_10000.sv_32.mrs_19.ks_30.csv" u 1:7 with lines lc 19 title "MRS=19", \
    "multiread.spp_10000.sv_32.mrs_20.ks_30.csv" u 1:7 with lines lc 20 title "MRS=20", \
    "multiread.spp_10000.sv_32.mrs_21.ks_30.csv" u 1:7 with lines lc 21 title "MRS=21", \
    "multiread.spp_10000.sv_32.mrs_22.ks_30.csv" u 1:7 with lines lc 22 title "MRS=22", \
    "multiread.spp_10000.sv_32.mrs_23.ks_30.csv" u 1:7 with lines lc 23 title "MRS=23", \
    "multiread.spp_10000.sv_32.mrs_24.ks_30.csv" u 1:7 with lines lc 24 title "MRS=24", \
    "multiread.spp_10000.sv_32.mrs_25.ks_30.csv" u 1:7 with lines lc 25 title "MRS=25", \
    "multiread.spp_10000.sv_32.mrs_26.ks_30.csv" u 1:7 with lines lc 26 title "MRS=26", \
    "multiread.spp_10000.sv_32.mrs_27.ks_30.csv" u 1:7 with lines lc 27 title "MRS=27", \
    "multiread.spp_10000.sv_32.mrs_28.ks_30.csv" u 1:7 with lines lc 28 title "MRS=28", \
    "multiread.spp_10000.sv_32.mrs_29.ks_30.csv" u 1:7 with lines lc 29 title "MRS=29", \
    "multiread.spp_10000.sv_32.mrs_30.ks_30.csv" u 1:7 with lines lc 30 title "MRS=30", \
    "multiread.spp_10000.sv_32.mrs_31.ks_30.csv" u 1:7 with lines lc 31 title "MRS=31", \
    "multiread.spp_10000.sv_32.mrs_32.ks_30.csv" u 1:7 with lines lc 32 title "MRS=32"

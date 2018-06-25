set title "Append Average Latency vs. Segment Size"
set xlabel "Segment Size (B)"
set ylabel "Append Average Latency (us)"
set key autotitle columnhead
set terminal postscript eps color
set output "append.spp_20000.es_20.eps"
plot "append.spp_20000.es_20.csv" u 1:2 with lines lc 1, \
      "append.spp_20000.es_20.model.csv" u 1:2 with lines lc 2

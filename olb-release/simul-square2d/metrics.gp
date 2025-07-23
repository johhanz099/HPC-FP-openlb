system("mkdir -p fig")
set datafile separator ","

# tiempo con p=1
stats "data/timings.csv" using ($1==1 ? $2 : 1/0) nooutput
T1 = STATS_min

#
# Gráfico 1: Speedup vs p 
#
set terminal pngcairo size 800,600 enhanced font "Arial,14"
set output "fig/speedup.png"
set title "Speedup vs Number of Processes"
set xlabel "Number of processes (p)"
set ylabel "Speedup = T₁ / Tₚ"
set grid
f_ideal(p) = p

plot \
  "data/timings.csv" using 1:(T1/$2) \
    with linespoints lw 2 pt 7 lc rgb "blue" title "Speedup medido", \
  f_ideal(x) \
    with lines lw 2 dt 2 lc rgb "gray" title "Speedup ideal"

set output

#
# Gráfico 2: Efficiency vs p 
#
set terminal pngcairo size 800,600 enhanced font "Arial,14"
set output "fig/efficiency.png"
set title "Efficiency vs Number of Processes"
set xlabel "Number of processes (p)"
set ylabel "Efficiency = (T₁/Tₚ)/p"
set grid

g_ideal(p) = 1.0
h_threshold(p) = 0.6

plot \
  "data/timings.csv" using 1:((T1/$2)/$1) \
    with linespoints lw 2 pt 7 lc rgb "red" title "Eficiencia medida", \
  g_ideal(x) \
    with lines lw 2 dt 2 lc rgb "black" title "Eficiencia ideal", \
  h_threshold(x) \
    with lines lw 2 dt 3 lc rgb "green" title "Umbral 60%"

set output
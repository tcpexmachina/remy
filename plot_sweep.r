library(ggplot2)

cycles = read.table("all_cycles_2.out", header=TRUE)
print(ggplot(cycles, aes(x=buffer, y=rewma, color=start)) +
  geom_point() +
    labs(x="Initial buffer size (packets)",
         y="EWMA of receive rate (ms)",
         color="Time to reach cycle (ms)") +
    scale_color_gradient(low="lightblue", high="darkblue"))
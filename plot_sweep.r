library(ggplot2)

cycles = read.table("cycles-small.out", header=TRUE)
print(ggplot(cycles, aes(x=buffer, y=rewma, color=factor(len))) +
  geom_point() +
    labs(x="Initial buffer size (packets)",
         y="EWMA of receive rate (ms)",
         color="Time to reach cycle (ms)"))
library(ggplot2)

data=read.table('9501.out', header=TRUE)
print(ggplot(data, aes(x=intersend/10000000, y=buffer/10000000)) +
        geom_path() +
        scale_y_continuous(breaks=seq(50, 51, by=1)) +
        labs(x='Time until next packet sent (ms)',
             y='Current buffer size (packets)'))
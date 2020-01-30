NBins <- 2^(16)

chn <- 0:(NBins-1)

c <- NBins*2/3
sd <- NBins/7

dat <- round(10000000*dnorm(x=chn,mean=c,sd=sd))

write.table(cbind(chn,dat),file="16k.dat",quote=F,col.names=F,row.names=F)

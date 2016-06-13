load_csv <- function(x) {
	require("data.table");
	d <- read.csv(header=FALSE, sep=' ', file=x);
	d <- data.table(d);
	return(d);
}


nust_summarize <- function (d) {
	#v = d[, list(ip = unique(V3), count = .N, type = V10), by = list(V3, V10)][, list(ip, count, type)];

	a <- d[substring(V3,1,3) != "10."];
	total <- dim(a)[1];
	b <- a[, list(ip = V3, count = .N, per = .N/total, type = V10), by = list(V3, V10)];
	c <- b[, list(ip, count, per, type)];

	return(c);
}

# Assumes the data is loaded in d
# d <- load_csv(d)

# Assumes the summary is stored in summary
# summary <- nust_summarize(d)

# All of the malicious IPs
# summary[type=="d9"]

# Ratio of malicious requests
# sum(summary[type=="d9", count]) / sum(summary[,count])

# Heavy hitters
# head(summary[order(-count),], 20)

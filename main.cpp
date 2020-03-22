#include <iostream>

bool parseNetInfo()
{
	FILE *fp = fopen("/proc/net/dev", "r");
	char buf[200];
	char ifname[20];
	unsigned long int r_bytes;
	unsigned long int t_bytes;
	unsigned long int r_packets;
	unsigned long int t_packets;

	// skip first two lines
	for (int i = 0; i < 2; i++) {
		fgets(buf, 200, fp);
	}

	while (fgets(buf, 200, fp)) {
		sscanf(buf, "%[^:]: %lu %lu %*lu %*lu %*lu %*lu %*lu %*lu %lu %lu",
				ifname, &r_bytes, &r_packets, &t_bytes, &t_packets);
		printf("%s: rbytes: %lu rpackets: %lu tbytes: %lu tpackets: %lu\n",
				ifname, r_bytes, r_packets, t_bytes, t_packets);
	}

	fclose(fp);
}

int main (int argc, char *argv[])
{
	parseNetInfo();

	return 0;
}

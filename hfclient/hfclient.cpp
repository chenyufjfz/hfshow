// hfclient.cpp : 定義主控台應用程式的進入點。
//

#include <stdio.h>
#include <stdlib.h>
#include "DisplayRpc.h"

DisplayRpc display;

int main()
{
	FILE *fp = fopen("LinkArray_1000_100_1_1600_0", "rt");
	char line[1000];
	int link_num, link_idx=0, power_num=1002*100;
	LineLink *link;
	int *power;

	if (fp == NULL)
		return -2;

	fgets(line, sizeof(line), fp);
	if (sscanf(line, "%d", &link_num) == 1) //read link number
		link = (LineLink*)malloc(link_num*sizeof(LineLink));
	else
		return -1;
	while (!feof(fp) && link_idx<link_num) {
		int ox, oy, oz, on, dx, dy, dz, dn;
		fgets(line, sizeof(line), fp);
		if (sscanf(line, "%d %d %d %d %d %d %d %d", &ox, &oy, &oz, &on, &dx, &dy, &dz, &dn) == 8) {//read link
			link[link_idx].startX = ox;
			link[link_idx].startY = oy;
			link[link_idx].endX = dx;
			link[link_idx].endY = dy;
			link[link_idx].endN = dn;
			link_idx++;
		}
	}
	fclose(fp);

	display.setLinkArray(link, link_idx, 1000, 100);
	power = (int*)malloc(power_num * sizeof(int));
	memset(power, 0, power_num * sizeof(int));
	srand(1);
	while (1) {
		for (int i = 0; i < power_num; i++) {
			int r = rand() % 100;
			int d = 0;
			if (r < 7)
				d = 1;
			if (r > 70)
				d = -1;
			power[i] += d;
			if (power[i] < 0)
				power[i] = 0;
		}
		display.showPower(power, power_num);
		Sleep(1000);
	}
	return 0;
}


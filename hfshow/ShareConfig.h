#ifndef SHARECONFIG_H
#define SHARECONFIG_H
#define SHARE_MEM_SIZE 0x8000000
#define PARA_SIZE 0x100
#define LINK_SIZE 0x6000000

struct ConfigPara {
	int width;
	int height;
	int link_updated;
	int power_updated;
	int link_num;
	int power_num;
};
#endif
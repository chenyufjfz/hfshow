#ifndef DISPLAYRPC_H
#define DISPLAYRPC_H
#include <windows.h>
#include <iostream>
#include <vector>
#include <string.h>
#include "linelink.h"

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

typedef struct {
	unsigned start, end;
} NodeLink;

/*
Normal use case
Display display
display.setLinkArray()
display.showPower
*/
class DisplayRpc {
protected:
	int width, height;
	HANDLE hmap, hmutex, hevent;
	LPVOID psharemem;
	volatile struct ConfigPara *ppara;
	NodeLink *plink;
	int *ppower;

public:
	DisplayRpc() {
		bool need_init = false;

		width = 0;
		height = 0;
		hmap = ::CreateFileMappingA(INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0, SHARE_MEM_SIZE,
			"FPGAShareMem");
		if (hmap == NULL) {
			std::cerr << "create file mapping error";
			::exit(-1);
		}			
		if (::GetLastError() != ERROR_ALREADY_EXISTS)
			need_init = true;

		psharemem = ::MapViewOfFile(hmap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (psharemem == NULL) {
			std::cerr << "map view fail";
			::exit(-2);
		}			

		hmutex = ::CreateMutexA(NULL, false, "FPGAShareMutex");
		if (hmutex == NULL) {
			std::cerr << "create mutex fail";
			::exit(-3);
		}			

		hevent = ::CreateEventA(NULL, FALSE, FALSE, "FPGAShareEvent");
		if (hevent == NULL) {
			std::cerr << "create event fail";
			::exit(-4);
		}
		ppara = (struct ConfigPara*) psharemem;
		plink = (NodeLink*)((char*)psharemem + PARA_SIZE);
		ppower = (int*)((char*)psharemem + PARA_SIZE + LINK_SIZE);

		if (need_init) {
			std::cout << "Displayer server not start\n";
			ppara->width = 0;
			ppara->height = 0;
			ppara->power_updated = 0;
			ppara->link_updated = 0;
			ppara->link_num = 0;
			ppara->power_num = 0;
		} else
			std::cout << "Displayer server already start\n";
	}

	~DisplayRpc() {
		::UnmapViewOfFile(psharemem);
		::CloseHandle(hmap);
		::CloseHandle(hmutex);
		::CloseHandle(hevent);
	}

	/*
	power Array has been changed, tell Display to show new power
	Input: power, the power array pointer, since FPGA slice have 4b output, power[i]<16
	Input: powerNumber, power array number
	*/
	void showPower(int *power, int powerNumber) {
		if (powerNumber > (width + 2)*height) {
			std::cerr << "power number exceed\n";
			return;
		}
			
		::WaitForSingleObject(hmutex, INFINITE);
		memcpy(ppower, power, powerNumber*sizeof(int));
		ppara->power_updated = 1;
		ppara->power_num = powerNumber;
		::ReleaseMutex(hmutex);
		::SetEvent(hevent);
	}

	/*
	linkArray has been changed, tell Display to reconfig linkArray
	Input: linkArray, the linkArray array pointer, Display only care for startX, startY, endX, endY. 
	       if startX==width, it is input port, if startX==width+1, it is ourput port
	Input: linkNumber, linkArray number
	Input: _width, Display width, not include i/o port.
	Input: _height, Display height
	*/
	void setLinkArray(LineLink * linkArray, int linkNumber, int _width, int _height) {
		int link_num = 0;
		width = _width;
		height = _height;	

		::WaitForSingleObject(hmutex, INFINITE);
		ppara->link_updated = 1;
		ppara->width = width;
		ppara->height = height;
		plink[-1].start = 0xffffffff; //set it to a invalid value, so that 
		for (int i = 0; i < linkNumber; i++) { 
			int end_port = 0;
			if (linkArray[i].endN <= 4)
				end_port = 3;
			else
				if (linkArray[i].endN <= 12)
					end_port = 2;
				else
					if (linkArray[i].endN <= 20)
						end_port = 1;
			plink[link_num].start = (linkArray[i].startX << 16) + (linkArray[i].startY<<3);
			plink[link_num].end = (linkArray[i].endX << 16) + (linkArray[i].endY<<3) + end_port;
			if (plink[link_num].start != plink[link_num - 1].start ||
				plink[link_num].end != plink[link_num - 1].end) ////remove overlap link
				link_num++;
		}
		ppara->link_num = link_num;
		::ReleaseMutex(hmutex);
	}
};
#endif
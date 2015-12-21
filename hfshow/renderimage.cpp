#include <QtWidgets>
#include "renderimage.h"


RenderImage::RenderImage(QObject *parent)
	: QThread(parent)
{
	bool need_init = false;

	width = 0;
	height = 0;
	xe = 48; 
	ye = 48;
	cd = 24;
	finish = false;
	hmap = ::CreateFileMappingA(INVALID_HANDLE_VALUE, 
		NULL, 
		PAGE_READWRITE,
		0, SHARE_MEM_SIZE,
		"FPGAShareMem");
	if (hmap == NULL) 
		::exit(-1);
	if (::GetLastError() != ERROR_ALREADY_EXISTS)
		need_init = true;

	psharemem = ::MapViewOfFile(hmap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (psharemem == NULL)
		::exit(-2);

	hmutex = ::CreateMutexA(NULL, false, "FPGAShareMutex");
	if (hmutex == NULL)
		::exit(-3);

	hevent = ::CreateEventA(NULL, FALSE, FALSE, "FPGAShareEvent");
	if (hevent == NULL)
		::exit(-4);

	ppara = (struct ConfigPara*) psharemem;
	plink = (NodeLink*)((char*)psharemem + PARA_SIZE);
	ppower = (int*) ((char*)psharemem + PARA_SIZE + LINK_SIZE);
	if (need_init) {
		ppara->width = 0;
		ppara->height = 0;
		ppara->power_updated = 0;
		ppara->link_updated = 0;
		ppara->link_num = 0;
		ppara->power_num = 0;
	}

	org_img.load("pic.png");
	neuron_img = org_img.scaled(cd, cd);
}

RenderImage::~RenderImage()
{
	::UnmapViewOfFile(psharemem);
	::CloseHandle(hmap);
	::CloseHandle(hmutex);
	::CloseHandle(hevent);
}

void RenderImage::end()
{
	finish = true;
}

void RenderImage::run()
{		
	vector<NodeLink> link;
	vector<int> power;
	int update_power=0, update_link=0;
	
	while(1) {
		::WaitForSingleObject(hevent, INFINITE);
		//Wakeup and check if link or power updated
		if (::WaitForSingleObject(hmutex, INFINITE) == WAIT_OBJECT_0) {
			update_link = ppara->link_updated;
			update_power = ppara->power_updated;
			if (width != ppara->width || height != ppara->height) {
				update_link = 1;
				update_power = 1;
				width = ppara->width;
				height = ppara->height;
			}

			if (update_link) { //read link
				link.assign(plink, plink + ppara->link_num);				
			}

			if (update_power) { // read power
				power.assign(ppower, ppower + ppara->power_num);
				power.resize((width + 2)*height, 0);
			}
			ppara->link_updated = 0;
			ppara->power_updated = 0;
			::ReleaseMutex(hmutex);
		}
		//if link or power updated, redraw image
		if (update_power || update_link) {
			QImage image((width + 3)*xe, height*ye, QImage::Format_RGB32);
			image.fill(Qt::black);
			QPainter p(&image);
			for (int i = 0; i <= width + 1; i++) {
				int x = (i == width) ? 1 : ((i == width + 1) ? width + 2 : i + 2);
				if (finish)
					break;
				for (int j = 0; j < height; j++) {
					int y = j + 1;
					int idx = i*height + j;
					//int color = (power[idx] >= COLOR_LEVEL) ? COLOR_LEVEL - 1 : power[idx];
					//color *= (256 / COLOR_LEVEL);
					
					if (i >= width) {
						p.setPen(QPen(Qt::yellow, 1));
						p.setBrush(QBrush(Qt::yellow));
						p.drawRect(QRect(x*xe - cd / 2, y*ye - cd / 2, cd, cd));
					}						
					else
						p.drawImage(QPointF(x*xe - cd / 2, y*ye - cd / 2), neuron_img);
				}
			}

			unsigned idx0 = 0xffffffff;
			for (int i = 0; i < link.size(); i++) {	
				unsigned startx = link[i].start >> 16;
				unsigned starty = link[i].start & 0xffff;
				unsigned endx = link[i].end >> 16;
				unsigned endy = link[i].end & 0xffff;
				unsigned idx = startx*height + starty;
				if (finish)
					break;
				if (idx != idx0) {
					int color = (power[idx] >= COLOR_LEVEL) ? COLOR_LEVEL - 1 : power[idx];
					color *= (256 / COLOR_LEVEL);
					p.setPen(QPen(QColor(255, 255, 255, color), 1));
				}
				if (startx >= width + 2 || endx >= width + 2 ||
					starty >= height || endy >= height) {
					p.save();
					p.setPen(QPen(QColor(255, 255, 255, 255), 1));
					p.drawText(0, 0, "some link number exceed range");
					p.restore();
				}
				else {
					startx = (startx == width) ? 1 : ((startx == width + 1) ? width + 2 : startx + 2);
					starty++;
					endx = (endx == width) ? 1 : ((endx == width + 1) ? width + 2 : endx + 2);
					endy++;
					p.drawLine(startx*xe, starty*ye, endx*xe, endy*ye);
					idx0 = idx;
				}
			}
			if (!finish)
				emit renderedImage(image);
		}
		//if mainthread require us quit, then quit
		if (finish)
			break;		
	}
}
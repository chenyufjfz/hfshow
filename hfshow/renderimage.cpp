#include <QtWidgets>
#include "renderimage.h"


RenderImage::RenderImage(QObject *parent)
	: QThread(parent)
{
	bool need_init = false;

	width = 0;
	height = 0;
	xe = 64; 
	ye = 64;
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
			QImage image_l((width / 2 + 1)*xe + xe / 2, height*ye, QImage::Format_RGB32);
			QImage image_r((width / 2 + 2)*xe, height*ye, QImage::Format_RGB32);
			image_l.fill(Qt::black);
			image_r.fill(Qt::black);
			QPainter pl(&image_l);
			QPainter pr(&image_r);

			for (int i = 0; i <= width / 2; i++) {
				int x = (i == width / 2) ? 1 : i + 2;
				if (finish)
					break;
				for (int j = 0; j < height; j++) {
					int y = j + 1;
					int idx = i*height + j;
					//int color = (power[idx] >= COLOR_LEVEL) ? COLOR_LEVEL - 1 : power[idx];
					//color *= (256 / COLOR_LEVEL);

					if (x == 1) {
						pl.setPen(QPen(Qt::yellow, 1));
						pl.setBrush(QBrush(Qt::yellow));
						pl.drawRect(QRect(x*xe - cd / 2, y*ye - cd / 2, cd, cd));
					}
					else
						pl.drawImage(QPointF(x*xe - cd / 2, y*ye - cd / 2), neuron_img);
				}
			}

			for (int i = width / 2 ; i <= width + 1; i++) {
				int x = (i == width + 1) ? width + 2 : i + 2;
				if (finish)
					break;
				for (int j = 0; j < height; j++) {
					int y = j + 1;
					int idx = i*height + j;
					if (x == width + 2) {
						pr.setPen(QPen(Qt::yellow, 1));
						pr.setBrush(QBrush(Qt::yellow));
						pr.drawRect(QRect(x*xe - cd / 2 - image_l.width(), y*ye - cd / 2, cd, cd));
					}
					else
						pr.drawImage(QPointF(x *xe - cd / 2 - image_l.width(), y*ye - cd / 2), neuron_img);
				}
			}
						
			for (int i = 0; i < link.size(); i++) {	
				int startx = link[i].start >> 16;
				int starty = (link[i].start & 0xffff)>>3;
				int endx = link[i].end >> 16;
				int endy = (link[i].end & 0xfff8)>>3;
				int endport = link[i].end & 7;
				unsigned idx = startx*height + starty;
				int color = (power[idx] >= COLOR_LEVEL) ? COLOR_LEVEL - 1 : power[idx];
				color *= (256 / COLOR_LEVEL);

				if (finish)
					break;				
				
				if (startx >= width + 2 || endx >= width + 2 ||
					starty >= height || endy >= height) {
					pl.save();
					pl.setPen(QPen(QColor(255, 255, 255, 255), 1));
					pl.drawText(0, 0, "some link number exceed range");
					pl.restore();
				}
				else {
					bool draw_left = false, draw_right = false;
					startx = (startx == width) ? 
						1 : 
						((startx == width + 1) ? width + 2 : startx + 2);
					starty++;
					startx = startx*xe;
					starty = starty*ye;					
					if (startx < image_l.width())
						draw_left = true;
					else 
						draw_right = true;						
											
					endx = (endx == width) ? 1 : ((endx == width + 1) ? width + 2 : endx + 2);
					endy++;
					endx = endx*xe;
					endy = endy*ye;
					if (endx < image_l.width())
						draw_left = true;
					else
						draw_right = true;

					endport = endport * 4 - 6;
					if (draw_left) {
						pl.setPen(QPen(QColor(255, 255, 255, color), 1));
						pl.drawLine(startx + cd / 2, starty, endx - cd / 2, endy + endport);
					}
					if (draw_right) {
						pr.setPen(QPen(QColor(255, 255, 255, color), 1));
						pr.drawLine(startx + cd / 2 - image_l.width(), starty, endx - cd / 2 - image_l.width(), endy + endport);
					}									
				}
			}

			if (!finish)
				emit renderedImage(image_l, image_r);
		}
		//if mainthread require us quit, then quit
		if (finish)
			break;		
	}
}
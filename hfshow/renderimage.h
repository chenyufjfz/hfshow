#ifndef RENDERIMAGE_H
#define RENDERIMAGE_H
#include <windows.h>
#include <QThread>
#include <vector>
#include <QtWidgets>
#include "ShareConfig.h"
using namespace std;
typedef struct {
	unsigned start, end;
} NodeLink;
#define COLOR_LEVEL 16

class FpgaShow;
class RenderImage : public QThread
{
	Q_OBJECT
		
public:
	RenderImage(QObject *parent = 0);
	~RenderImage();
	void end();

signals:
	void renderedImage(const QImage &image_l, const QImage &image_r);

protected:
	void run() Q_DECL_OVERRIDE;	

private:
	int width, height, xe, ye, cd; //xe>2*cd
	HANDLE hmap, hmutex, hevent;
	LPVOID psharemem;
	bool finish;
	volatile struct ConfigPara *ppara;
	NodeLink *plink;
	int *ppower;
	friend class FpgaShow;
	QImage org_img, neuron_img;
};

#endif // RENDERIMAGE_H

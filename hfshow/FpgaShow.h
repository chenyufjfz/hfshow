#ifndef FPGASHOW_H
#define FPGASHOW_H
#include <QPixmap>
#include <QWidget>
#include <vector>
#include "renderimage.h"

using namespace std;
#define COLOR_LEVEL 16
class FpgaShow : public QWidget
{
	Q_OBJECT

public:
	FpgaShow(QWidget *parent);
	~FpgaShow();
	void setscale(float s);
	float getscale();
	
signals:
	void MouseChange(QPoint point);

protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void closeEvent(QCloseEvent * event);
	bool mouse_press;
	float scale;
	RenderImage render;
	QPixmap pixmap;
protected slots:
	void updatePixmap(const QImage &image);
};

#endif // FPGASHOW_H

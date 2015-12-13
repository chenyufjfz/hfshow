#ifndef HFSHOW_H
#define HFSHOW_H

#include <QtWidgets/QMainWindow>
#include "ui_hfshow.h"
#include "FpgaShow.h"

QT_BEGIN_NAMESPACE
class QScrollArea;
class QLabel;
QT_END_NAMESPACE

class hfshow : public QMainWindow
{
	Q_OBJECT

public:
	hfshow(QWidget *parent = 0);
	~hfshow();
protected:
	float scale;
	FpgaShow * fpga_show;
	QScrollArea *scroll_view;
	QLabel *status_label;
	bool mouse_press;
	QPointF mouse_pos;

protected slots:
	void zoomin();
	void zoomout();
	void zoomfit();
	void mouse_change(QPoint pos);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);	
private:
	Ui::hfshowClass ui;
};

#endif // HFSHOW_H

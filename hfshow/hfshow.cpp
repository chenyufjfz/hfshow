#include <QtWidgets>
#include "hfshow.h"

hfshow::hfshow(QWidget *parent)
	: QMainWindow(parent)
{
	vector<NodeLink> link;
	ui.setupUi(this);
	fpga_show = new FpgaShow(NULL);
	scale = fpga_show->getscale();

	scroll_view = new QScrollArea;
	scroll_view->setBackgroundRole(QPalette::Dark);
	scroll_view->setWidget(fpga_show);
	setCentralWidget(scroll_view);

	status_label = new QLabel;
	status_label->setMinimumSize(200, 20);
	ui.statusBar->addWidget(status_label);

	connect(ui.actionZoomin, SIGNAL(triggered()), this, SLOT(zoomin()));
	connect(ui.actionZoomout, SIGNAL(triggered()), this, SLOT(zoomout()));
	connect(ui.actionWhole, SIGNAL(triggered()), this, SLOT(zoomfit()));
	connect(fpga_show, SIGNAL(MouseChange(QPoint)), this, SLOT(mouse_change(QPoint)));

	mouse_press = false;
}

hfshow::~hfshow()
{
}

void hfshow::zoomin()
{	
	QPoint current_pos = mapFromGlobal(cursor().pos());
	double x = (double)(scroll_view->horizontalScrollBar()->value() + current_pos.x()) / scale;
	double y = (double)(scroll_view->verticalScrollBar()->value() + current_pos.y()) / scale;
	fpga_show->setscale(fpga_show->getscale() *2);
	scale = fpga_show->getscale();
	scroll_view->horizontalScrollBar()->setValue(x*scale - current_pos.x());
	scroll_view->verticalScrollBar()->setValue(y*scale - current_pos.y());
}

void hfshow::zoomout()
{
	QPointF current_pos = mapFromGlobal(cursor().pos());
	double x = (double)(scroll_view->horizontalScrollBar()->value() + current_pos.x()) / scale;
	double y = (double)(scroll_view->verticalScrollBar()->value() + current_pos.y()) / scale;
	fpga_show->setscale(fpga_show->getscale() / 2);
	scale = fpga_show->getscale();
	scroll_view->horizontalScrollBar()->setValue(x*scale - current_pos.x());
	scroll_view->verticalScrollBar()->setValue(y*scale - current_pos.y());
}

void hfshow::zoomfit()
{

}

void hfshow::mouse_change(QPoint pos)
{
	char s[100];
	sprintf(s, "x:%d,y:%d", pos.x(), pos.y());	
	status_label->setText(s);
	//mouse_pos->repaint();
}

void hfshow::mousePressEvent(QMouseEvent *event)
{
	mouse_press = true;
	mouse_pos = event->pos();
	setCursor(Qt::ClosedHandCursor);
	QWidget::mousePressEvent(event);
}

void hfshow::mouseReleaseEvent(QMouseEvent *event)
{
	mouse_press = false;
	setCursor(Qt::ArrowCursor);
	QWidget::mouseReleaseEvent(event);
}

void hfshow::mouseMoveEvent(QMouseEvent *event)
{
	if (mouse_press) {
		QPointF delta = event->pos() - mouse_pos;
		scroll_view->horizontalScrollBar()->setValue(scroll_view->horizontalScrollBar()->value() - delta.x());
		scroll_view->verticalScrollBar()->setValue(scroll_view->verticalScrollBar()->value() - delta.y());
		mouse_pos = event->pos();
	}	
	QWidget::mouseMoveEvent(event);
}

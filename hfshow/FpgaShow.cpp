#include <QtWidgets>
#include "FpgaShow.h"
#include <math.h>

FpgaShow::FpgaShow(QWidget *parent)
	: QWidget(parent)
{
	scale = 1;

	mouse_press = false;
	setMouseTracking(true);
	connect(&render, SIGNAL(renderedImage(QImage)), this, SLOT(updatePixmap(QImage)));
	render.start();
	resize(600, 400);
	/*
	setAutoFillBackground(true);
	QPalette pal(palette());
	pal.setColor(QPalette::Window, Qt::black);
	setPalette(pal);*/
}

FpgaShow::~FpgaShow()
{
	render.end();
	render.wait();
}

void FpgaShow::setscale(float s)
{
	if (s <= 2.01 && s >= 0.124) {
		scale = s;
		int neww, newh;
		if (pixmap.isNull()) {
			neww = 600;
			newh = 400;
		}
		else {
			neww = int(pixmap.width() * scale);
			newh = int(pixmap.height() * scale);
		}
		
		resize(neww, newh);
		update();
	}		
}

float FpgaShow::getscale()
{
	return scale;
}

void FpgaShow::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	if (pixmap.isNull()) {
		painter.setPen(Qt::white);
		painter.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
		return;
	}

	painter.save();	
	painter.scale(scale, scale);
	painter.drawPixmap(0, 0, pixmap);
	painter.restore();
	
}

void FpgaShow::mousePressEvent(QMouseEvent *event)
{
	mouse_press = true;
	QWidget::mousePressEvent(event);
}
void FpgaShow::mouseReleaseEvent(QMouseEvent *event)
{
	mouse_press = false;
	QWidget::mouseReleaseEvent(event);
}

void FpgaShow::mouseMoveEvent(QMouseEvent *event)
{
	if (!mouse_press) {
		int y = (event->localPos().y()+render.ye*scale/2) / (scale*render.ye);
		int x = (event->localPos().x()+render.xe*scale/2) / (scale*render.xe);
		emit MouseChange(QPoint(x, y));
	}
	QWidget::mouseMoveEvent(event);
}

void FpgaShow::closeEvent(QCloseEvent * event)
{
	render.end();
	render.wait();
}

void FpgaShow::updatePixmap(const QImage &image)
{
	pixmap = QPixmap::fromImage(image);
	int neww = int(pixmap.width() * scale);
	int newh = int(pixmap.height() * scale);
	resize(neww, newh);
	update();
}
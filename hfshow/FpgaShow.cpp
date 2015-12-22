#include <QtWidgets>
#include "FpgaShow.h"
#include <math.h>

FpgaShow::FpgaShow(QWidget *parent)
	: QWidget(parent)
{
	scale = 0.5;

	mouse_press = false;
	setMouseTracking(true);
	connect(&render, SIGNAL(renderedImage(QImage, QImage)), this, SLOT(updatePixmap(QImage, QImage)));
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
	if (s <= 1.01 && s >= 0.062) {
		scale = s;
		int neww, newh;
		if (img_l.isNull()) {
			neww = 600;
			newh = 400;
		}
		else {
			neww = int(width * scale);
			newh = int(height * scale);
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
	if (img_l.isNull()) {
		painter.setPen(Qt::white);
		painter.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
		return;
	}

	painter.save();	
	painter.scale(scale, scale);
	painter.drawImage(0, 0, img_l);
	painter.drawImage(img_l.width(), 0, img_r);
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

void FpgaShow::updatePixmap(const QImage &image_l, const QImage &image_r)
{
	
	width = image_l.width() +image_r.width();
	height = image_l.height();
	img_l = image_l;
	img_r = image_r;
	int neww = int(width * scale);
	int newh = int(height * scale);
	resize(neww, newh);
	update();
}
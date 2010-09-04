#include "zoomwidget.hpp"
#include "ui_zoomwidget.h"

#include <QPainter>
#include <QDesktopWidget>
#include <QMouseEvent>

zoomwidget::zoomwidget(QWidget *parent) :
		QGLWidget(parent),
		ui(new Ui::zoomwidget)
{
	ui->setupUi(this);
	setMouseTracking(true);

	this->grab_desktop();

	this->is_dragging = true;

	this->pixmap_pos = QPoint(0, 0);
	this->pixmap_size = this->pixmap.size();
	shift_sensivity = 2;
	scale_sensivity = 0.1f;
}

zoomwidget::~zoomwidget()
{
	delete ui;
}

void zoomwidget::paintEvent(QPaintEvent *event)
{
	QPainter p;

	p.begin(this);
	p.drawPixmap(this->pixmap_pos.x(), this->pixmap_pos.y(),
		     this->pixmap_size.width(), this->pixmap_size.height(), this->pixmap);
	p.end();
}

void zoomwidget::mousePressEvent(QMouseEvent *event)
{
	this->last_mouse_pos = event->pos();
}

void zoomwidget::mouseReleaseEvent(QMouseEvent *event)
{
}

void zoomwidget::mouseMoveEvent(QMouseEvent *event)
{
	if (this->is_dragging) {
		QPoint delta = event->pos() - this->last_mouse_pos;

		this->shift_pixmap(delta);
		this->check_pixmap_pos();

		this->update();
	}

	this->last_mouse_pos = event->pos();
}

void zoomwidget::wheelEvent(QWheelEvent *event)
{
	int sign = event->delta() / abs(event->delta());

	static float pixmap_scale = 1.0f;
	pixmap_scale += sign * scale_sensivity;
	if (pixmap_scale < 1.0f) {
		pixmap_scale = 1.0f;
	}

	this->scale_pixmap_at(pixmap_scale, event->pos());
	this->check_pixmap_pos();

	this->update();
}

void zoomwidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		QApplication::quit();
	} else if (event->key() == Qt::Key_Control) {
		this->is_dragging = false;
	}
}

void zoomwidget::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Control) {
		this->is_dragging = true;
	}
}

void zoomwidget::grab_desktop()
{
	this->pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
}

void zoomwidget::shift_pixmap(const QPoint delta)
{
	this->pixmap_pos -= delta * shift_sensivity;
}

void zoomwidget::scale_pixmap_at(float scale, const QPoint pos)
{
	int old_w = this->pixmap_size.width();
	int old_h = this->pixmap_size.height();

	int new_w = this->pixmap.width() * scale;
	int new_h = this->pixmap.height() * scale;
	this->pixmap_size = QSize(new_w, new_h);

	int dw = new_w - old_w;
	int dh = new_h - old_h;

	int cur_x = pos.x() + abs(this->pixmap_pos.x());
	int cur_y = pos.y() + abs(this->pixmap_pos.y());

	float cur_px = -((float)cur_x / old_w);
	float cur_py = -((float)cur_y / old_h);

	this->pixmap_pos.setX(this->pixmap_pos.x() + dw*cur_px);
	this->pixmap_pos.setY(this->pixmap_pos.y() + dh*cur_py);
}

void zoomwidget::check_pixmap_pos()
{
	if (pixmap_pos.x() > 0) {
		pixmap_pos.setX(0);
	} else if ((pixmap_size.width() + pixmap_pos.x()) < this->width()) {
		pixmap_pos.setX(this->width() - pixmap_size.width());
	}

	if (pixmap_pos.y() > 0) {
		pixmap_pos.setY(0);
	} else if ((pixmap_size.height() + pixmap_pos.y()) < this->height()) {
		pixmap_pos.setY(this->height() - pixmap_size.height());
	}
}

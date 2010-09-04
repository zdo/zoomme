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

	for (int i = 0; i < this->todraw_rects.size(); ++i) {
		p.setPen(this->todraw_rects.at(i).pen);
		p.drawRect(this->pixmap_pos.x() + this->todraw_rects.at(i).rect.x()*scale,
			   this->pixmap_pos.y() + this->todraw_rects.at(i).rect.y()*scale,
			   this->todraw_rects.at(i).rect.width()*scale,
			   this->todraw_rects.at(i).rect.height()*scale);
	}

	if (!this->is_dragging) {
		p.setPen(this->active_pen);
		p.drawRect(this->pixmap_pos.x() + this->active_rect.x()*scale,
			   this->pixmap_pos.y() + this->active_rect.y()*scale,
			   this->active_rect.width()*scale,
			   this->active_rect.height()*scale);
	}

	p.end();
}

void zoomwidget::mousePressEvent(QMouseEvent *event)
{
	this->last_mouse_pos = event->pos();

	if (event->modifiers() & Qt::ControlModifier) {
		this->is_dragging = false;

		draw_point_start = (event->pos() - this->pixmap_pos)/scale;
		active_rect.setTopLeft(draw_point_start);
		active_rect.setBottomRight(draw_point_start);
	}
}

void zoomwidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->modifiers() & Qt::ControlModifier) {
		draw_point_end = (event->pos() - this->pixmap_pos)/scale;
		active_rect.setBottomRight(draw_point_end);

		rect_data rd;
		rd.rect = QRect(draw_point_start, draw_point_end);
		rd.pen = active_pen;
		this->todraw_rects.append(rd);

		this->is_dragging = true;

		this->update();
	}
}

void zoomwidget::mouseMoveEvent(QMouseEvent *event)
{
	if (this->is_dragging) {
		QPoint delta = event->pos() - this->last_mouse_pos;

		this->shift_pixmap(delta);
		this->check_pixmap_pos();
	} else if (event->modifiers() & Qt::ControlModifier) {
		draw_point_end = (event->pos() - this->pixmap_pos)/scale;
		active_rect.setBottomRight(draw_point_end);
	}

	this->last_mouse_pos = event->pos();
	this->update();
}

void zoomwidget::wheelEvent(QWheelEvent *event)
{
	int sign = event->delta() / abs(event->delta());

	scale += sign * scale_sensivity;
	if (scale < 1.0f) {
		scale = 1.0f;
	}

	this->scale_pixmap_at(event->pos());
	this->check_pixmap_pos();

	this->update();
}

void zoomwidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		QApplication::quit();
	} else if (event->modifiers() == Qt::ControlModifier) {
		int key = event->key();
		if ((key >= Qt::Key_1) && (key <= Qt::Key_9)) {
			this->active_pen.setWidth(key - Qt::Key_0);
		} else if (key == Qt::Key_R) {
			this->active_pen.setColor(QColor(255, 0, 0));
		} else if (key == Qt::Key_G) {
			this->active_pen.setColor(QColor(0, 255, 0));
		}

		this->update();
	}
}

void zoomwidget::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Control) {
		//this->is_dragging = true;
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

void zoomwidget::scale_pixmap_at(const QPoint pos)
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

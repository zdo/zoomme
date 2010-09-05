#include "zoomwidget.hpp"
#include "ui_zoomwidget.h"

#include <QPainter>
#include <QDesktopWidget>
#include <QMouseEvent>

ZoomWidget::ZoomWidget(QWidget *parent) :
		QGLWidget(parent),
		ui(new Ui::zoomwidget)
{
	ui->setupUi(this);
	setMouseTracking(true);

	_state = STATE_MOVING;

	_desktopPixmapPos = QPoint(0, 0);
	_desktopPixmapSize = QApplication::desktop()->size();
	_desktopPixmapScale = 1.0f;

	_shiftMultiplier = 2;
	_scaleSensivity = 0.1f;

	_drawMode = DRAWMODE_LINE;
}

ZoomWidget::~ZoomWidget()
{
	delete ui;
}

void ZoomWidget::paintEvent(QPaintEvent *event)
{
	QPainter p;

	p.begin(this);

	// Draw desktop pixmap.
	p.drawPixmap(_desktopPixmapPos.x(), _desktopPixmapPos.y(),
		     _desktopPixmapSize.width(), _desktopPixmapSize.height(),
		     _desktopPixmap);

	// Draw user rects.
	for (int i = 0; i < _userRects.size(); ++i) {
		p.setPen(_userRects.at(i).pen);
		int x = _desktopPixmapPos.x() + _userRects.at(i).startPoint.x()*_desktopPixmapScale;
		int y = _desktopPixmapPos.y() + _userRects.at(i).startPoint.y()*_desktopPixmapScale;
		int w = (_userRects.at(i).endPoint.x() - _userRects.at(i).startPoint.x())*_desktopPixmapScale;
		int h = (_userRects.at(i).endPoint.y() - _userRects.at(i).startPoint.y())*_desktopPixmapScale;
		p.drawRect(x, y, w, h);
	}

	// Draw user lines.
	for (int i = 0; i < _userLines.size(); ++i) {
		p.setPen(_userLines.at(i).pen);
		int x = _desktopPixmapPos.x() + _userLines.at(i).startPoint.x()*_desktopPixmapScale;
		int y = _desktopPixmapPos.y() + _userLines.at(i).startPoint.y()*_desktopPixmapScale;
		int w = (_userLines.at(i).endPoint.x() - _userLines.at(i).startPoint.x())*_desktopPixmapScale;
		int h = (_userLines.at(i).endPoint.y() - _userLines.at(i).startPoint.y())*_desktopPixmapScale;
		p.drawLine(x, y, x+w, y+h);
	}

	// Draw active rect.
	if (_state == STATE_DRAWING) {
		p.setPen(_activePen);

		int x = _desktopPixmapPos.x() + _startDrawPoint.x()*_desktopPixmapScale;
		int y = _desktopPixmapPos.y() + _startDrawPoint.y()*_desktopPixmapScale;
		int width = (_endDrawPoint.x() - _startDrawPoint.x())*_desktopPixmapScale;
		int height = (_endDrawPoint.y() - _startDrawPoint.y())*_desktopPixmapScale;

		if (_drawMode == DRAWMODE_RECT) {
			p.drawRect(x, y, width, height);
		} else if (_drawMode == DRAWMODE_LINE) {
			p.drawLine(x, y, width + x, height + y);
		}
	}

	p.end();
}

void ZoomWidget::mousePressEvent(QMouseEvent *event)
{
	_lastMousePos = event->pos();

	_state = STATE_DRAWING;

	_startDrawPoint = (event->pos() - _desktopPixmapPos)/_desktopPixmapScale;
	_endDrawPoint = _startDrawPoint;
}

void ZoomWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (_state == STATE_DRAWING) {
		_endDrawPoint = (event->pos() - _desktopPixmapPos)/_desktopPixmapScale;

		UserObjectData data;
		data.pen = _activePen;
		data.startPoint = _startDrawPoint;
		data.endPoint = _endDrawPoint;
		if (_drawMode == DRAWMODE_LINE) {
			_userLines.append(data);
		} else if (_drawMode == DRAWMODE_RECT) {
			_userRects.append(data);
		}

		_state = STATE_MOVING;
		update();
	}
}

void ZoomWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (_state == STATE_MOVING) {
		QPoint delta = event->pos() - _lastMousePos;

		shiftPixmap(delta);
		checkPixmapPos();
	} else if (_state == STATE_DRAWING) {
		_endDrawPoint = (event->pos() - _desktopPixmapPos)/_desktopPixmapScale;
	}

	_lastMousePos = event->pos();
	update();
}

void ZoomWidget::wheelEvent(QWheelEvent *event)
{
	if (_state == STATE_MOVING) {
		int sign = event->delta() / abs(event->delta());

		_desktopPixmapScale += sign * _scaleSensivity;
		if (_desktopPixmapScale < 1.0f) {
			_desktopPixmapScale = 1.0f;
		}

		scalePixmapAt(event->pos());
		checkPixmapPos();

		update();
	}
}

void ZoomWidget::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();

	if (key == Qt::Key_Escape) {
		QApplication::quit();
	} else if ((key >= Qt::Key_1) && (key <= Qt::Key_9)) {
		_activePen.setWidth(key - Qt::Key_0);
	} else if (key == Qt::Key_R) {
		_activePen.setColor(QColor(255, 0, 0));
	} else if (key == Qt::Key_G) {
		_activePen.setColor(QColor(0, 255, 0));
	} else if (key == Qt::Key_B) {
		_activePen.setColor(QColor(0, 0, 255));
	} else if (key == Qt::Key_C) {
		_activePen.setColor(QColor(0, 255, 255));
	} else if (key == Qt::Key_M) {
		_activePen.setColor(QColor(255, 0, 255));
	} else if (key == Qt::Key_Y) {
		_activePen.setColor(QColor(255, 255, 0));
	} else if (key == Qt::Key_W) {
		_activePen.setColor(QColor(255, 255, 255));
	} else if (key == Qt::Key_Q) {
		_userRects.clear();
		_userLines.clear();
		_state = STATE_MOVING;
	} else if (key == Qt::Key_Z) {
		_drawMode = DRAWMODE_LINE;
	} else if (key == Qt::Key_X) {
		_drawMode = DRAWMODE_RECT;
	}

	update();
}

void ZoomWidget::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Control) {
		//_is_dragging = true;
	}
}

void ZoomWidget::grabDesktop()
{
	_desktopPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
}

void ZoomWidget::shiftPixmap(const QPoint delta)
{
	_desktopPixmapPos -= delta * _shiftMultiplier;
}

void ZoomWidget::scalePixmapAt(const QPoint pos)
{
	int old_w = _desktopPixmapSize.width();
	int old_h = _desktopPixmapSize.height();

	int new_w = _desktopPixmap.width() * _desktopPixmapScale;
	int new_h = _desktopPixmap.height() * _desktopPixmapScale;
	_desktopPixmapSize = QSize(new_w, new_h);

	int dw = new_w - old_w;
	int dh = new_h - old_h;

	int cur_x = pos.x() + abs(_desktopPixmapPos.x());
	int cur_y = pos.y() + abs(_desktopPixmapPos.y());

	float cur_px = -((float)cur_x / old_w);
	float cur_py = -((float)cur_y / old_h);

	_desktopPixmapPos.setX(_desktopPixmapPos.x() + dw*cur_px);
	_desktopPixmapPos.setY(_desktopPixmapPos.y() + dh*cur_py);
}

void ZoomWidget::checkPixmapPos()
{
	if (_desktopPixmapPos.x() > 0) {
		_desktopPixmapPos.setX(0);
	} else if ((_desktopPixmapSize.width() + _desktopPixmapPos.x()) < width()) {
		_desktopPixmapPos.setX(width() - _desktopPixmapSize.width());
	}

	if (_desktopPixmapPos.y() > 0) {
		_desktopPixmapPos.setY(0);
	} else if ((_desktopPixmapSize.height() + _desktopPixmapPos.y()) < height()) {
		_desktopPixmapPos.setY(height() - _desktopPixmapSize.height());
	}
}

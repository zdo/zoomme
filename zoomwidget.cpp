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
		p.drawRect(_desktopPixmapPos.x() + _userRects.at(i).rect.x()*_desktopPixmapScale,
			   _desktopPixmapPos.y() + _userRects.at(i).rect.y()*_desktopPixmapScale,
			   _userRects.at(i).rect.width()*_desktopPixmapScale,
			   _userRects.at(i).rect.height()*_desktopPixmapScale);
	}

	// Draw active rect.
	if (_state == STATE_DRAWING) {
		p.setPen(_activePen);
		p.drawRect(_desktopPixmapPos.x() + _activeRect.x()*_desktopPixmapScale,
			   _desktopPixmapPos.y() + _activeRect.y()*_desktopPixmapScale,
			   _activeRect.width()*_desktopPixmapScale,
			   _activeRect.height()*_desktopPixmapScale);
	}

	p.end();
}

void ZoomWidget::mousePressEvent(QMouseEvent *event)
{
	_lastMousePos = event->pos();

	if (event->modifiers() & Qt::ControlModifier) {
		_state = STATE_DRAWING;

		_startDrawPoint = (event->pos() - _desktopPixmapPos)/_desktopPixmapScale;
		_activeRect.setTopLeft(_startDrawPoint);
		_activeRect.setBottomRight(_startDrawPoint);
	}
}

void ZoomWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if ((_state == STATE_DRAWING) && (event->modifiers() & Qt::ControlModifier)) {
		_endDrawPoint = (event->pos() - _desktopPixmapPos)/_desktopPixmapScale;
		_activeRect.setBottomRight(_endDrawPoint);

		RectData rd;
		rd.rect = QRect(_startDrawPoint, _endDrawPoint);
		rd.pen = _activePen;
		_userRects.append(rd);

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
	} else if ((_state == STATE_DRAWING) && (event->modifiers() & Qt::ControlModifier)) {
		_endDrawPoint = (event->pos() - _desktopPixmapPos)/_desktopPixmapScale;
		_activeRect.setBottomRight(_endDrawPoint);
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
	} else if (key == Qt::Key_Q) {
		_userRects.clear();
		_userLines.clear();
		_state = STATE_MOVING;
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

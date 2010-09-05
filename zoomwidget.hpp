#ifndef ZOOMWIDGET_HPP
#define ZOOMWIDGET_HPP

#include <QGLWidget>

namespace Ui {
	class zoomwidget;
}

// User data structs.
struct RectData {
	QRect rect;
	QPen pen;
};
struct LineData {
	QLine line;
	QPen pen;
};

enum ZoomWidgetState {
	STATE_MOVING,
	STATE_DRAWING,
};

class ZoomWidget : public QGLWidget
{
	Q_OBJECT

public:
	explicit ZoomWidget(QWidget *parent = 0);
	~ZoomWidget();

	void grabDesktop();

protected:
	virtual void paintEvent(QPaintEvent *event);

	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

	virtual void wheelEvent(QWheelEvent *event);

	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);

private:
	Ui::zoomwidget *ui;

	// Desktop pixmap properties.
	QPixmap		_desktopPixmap;
	QPoint		_desktopPixmapPos;
	QSize		_desktopPixmapSize;
	float		_desktopPixmapScale;

	// User objects.
	QVector<RectData> _userRects;
	QVector<LineData> _userLines;

	// Moving properties.
	int		_shiftMultiplier;
	float		_scaleSensivity;


	ZoomWidgetState	_state;
	QPoint		_lastMousePos;


	// Drawing properties.
	QPoint	_startDrawPoint;
	QPoint	_endDrawPoint;
	QPen	_activePen;
	QRect	_activeRect;


	void shiftPixmap(const QPoint delta);
	void scalePixmapAt(const QPoint pos);

	void checkPixmapPos();
};

#endif // ZOOMWIDGET_HPP

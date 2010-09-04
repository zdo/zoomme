#ifndef ZOOMWIDGET_HPP
#define ZOOMWIDGET_HPP

#include <QGLWidget>

namespace Ui {
	class zoomwidget;
}

struct rect_data {
	QRect rect;
	QPen pen;
};

struct line_data {
	QLine line;
	QPen pen;
};

class zoomwidget : public QGLWidget
{
	Q_OBJECT

public:
	explicit zoomwidget(QWidget *parent = 0);
	~zoomwidget();

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

	QPixmap pixmap; /* pixmap to draw */

	QVector<rect_data> todraw_rects;
	QVector<line_data> todraw_lines;

	QPoint pixmap_pos;
	QSize pixmap_size;
	float scale;

	bool is_dragging;
	QPoint last_mouse_pos;

	int shift_sensivity;
	float scale_sensivity;

	QPoint draw_point_start;
	QPoint draw_point_end;
	QPen active_pen;
	QRect active_rect;


	void grab_desktop();
	void shift_pixmap(const QPoint delta);
	void scale_pixmap_at(const QPoint pos);

	void check_pixmap_pos();
};

#endif // ZOOMWIDGET_HPP

#include <QtGui/QApplication>
#include "zoomwidget.hpp"
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	zoomwidget w;

	//w.setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
	//w.resize(QApplication::desktop()->size());
	//w.showFullScreen();
	w.show();

	return a.exec();
}

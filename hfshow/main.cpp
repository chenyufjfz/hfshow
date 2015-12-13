#include "hfshow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	hfshow w;
	w.show();
	return a.exec();
}

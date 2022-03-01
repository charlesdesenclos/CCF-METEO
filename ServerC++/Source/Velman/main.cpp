#include <QtCore/QCoreApplication>
#include "velman.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	new velman(1234);

	return a.exec();
}

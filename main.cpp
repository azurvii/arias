/*
 * main.cpp
 *
 *  Created on: May 27, 2010
 *      Author: Vincent
 */

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char ** argv) {
	QApplication a(argc, argv);
	QApplication::setOrganizationName("Yaffe Lab");
	QApplication::setOrganizationDomain("arias.googlecode.com");
	QApplication::setApplicationName("ARIAS");
	MainWindow w;
	w.show();
	return a.exec();
}

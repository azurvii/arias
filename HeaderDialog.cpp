/*
 * HeaderDialog.cpp
 *
 *  Created on: Jun 7, 2010
 *      Author: Vincent
 */

#include "HeaderDialog.h"

HeaderDialog::HeaderDialog(QWidget * parent) :
	QDialog(parent) {
	ui.setupUi(this);
}

HeaderDialog::~HeaderDialog() {
}

QStringList HeaderDialog::getHeaders() const {
	QStringList headers;
	headers << ui.rowLabelEdit->text();
	headers << ui.colLabelEdit->text();
	return headers;
}

void HeaderDialog::setRowHeaders(const QStringList & headers) {
	ui.rowLabelEdit->setText(headers.join(" "));
}

void HeaderDialog::setColumnHeaders(const QStringList & headers) {
	ui.colLabelEdit->setText(headers.join(" "));
}

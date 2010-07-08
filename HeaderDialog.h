/*
 * HeaderDialog.h
 *
 *  Created on: Jun 7, 2010
 *      Author: Vincent
 */

#ifndef HEADERDIALOG_H_
#define HEADERDIALOG_H_

#include "ui_HeaderDialog.h"

class HeaderDialog: public QDialog {
Q_OBJECT
public:
	HeaderDialog(QWidget * parent = 0);
	virtual ~HeaderDialog();

public:
	QStringList getHeaders() const;

public slots:
	void setRowHeaders(const QStringList & headers);
	void setColumnHeaders(const QStringList & headers);

private:
	Ui::HeaderDialog ui;
};

#endif /* HEADERDIALOG_H_ */

/*
 * DoubleDelegate.cpp
 *
 *  Created on: Apr 28, 2010
 *      Author: Vincent
 */

#include "DoubleDelegate.h"
#include <limits.h>
#include <QLineEdit>

DoubleDelegate::DoubleDelegate(QObject *parent) :
	QItemDelegate(parent) {
}

DoubleDelegate::~DoubleDelegate() {
}

QWidget *DoubleDelegate::createEditor(QWidget *parent,
		const QStyleOptionViewItem &, const QModelIndex &) const {
	QLineEdit *editor = new QLineEdit(parent);
	return editor;
}

void DoubleDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
	double value = index.model()->data(index, Qt::EditRole).toDouble();
	QLineEdit *lineEdit = static_cast<QLineEdit*> (editor);
	lineEdit->setText(QString::number(value, 'g', 15));
}

void DoubleDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
		const QModelIndex &index) const {
	QLineEdit *lineEdit = static_cast<QLineEdit*> (editor);
	double value = lineEdit->text().toDouble();
	model->setData(index, value, Qt::EditRole);
}

void DoubleDelegate::updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &) const {
	editor->setGeometry(option.rect);
}

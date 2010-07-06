/*
 * DoubleDelegate.h
 *
 *  Created on: Apr 28, 2010
 *      Author: Vincent
 *
 *  Provide a delegate in model-view framework for editing table values
 *
 */

#ifndef DOUBLEDELEGATE_H_
#define DOUBLEDELEGATE_H_

#include <QItemDelegate>

class DoubleDelegate: public QItemDelegate {
Q_OBJECT
public:
	DoubleDelegate(QObject *parent = 0);
	virtual ~DoubleDelegate();

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
			const QModelIndex &index) const;
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
			const QModelIndex &index) const;
	void updateEditorGeometry(QWidget *editor,
			const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif /* DOUBLEDELEGATE_H_ */

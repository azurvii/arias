/*
 * Matrix.h
 *
 *  Created on: May 27, 2010
 *      Author: Vincent
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <QList>
#include <QPoint>

template<typename ElementType>
class Matrix {
public:
	typedef QListIterator<ElementType> Iterator;

public:
	Matrix(qint32 rows = 0, qint32 columns = 0) {
		initMatrix(rows, columns);
	}

	virtual ~Matrix() {
	}

	ElementType & getValue(qint32 row, qint32 column) {
		return array[row * columns + column];
	}

	const ElementType & getValue(qint32 row, qint32 column) const {
		return array[row * columns + column];
	}

	void setValue(qint32 row, qint32 column, const ElementType & value) {
		getValue(row, column) = value;
	}

	/**
	 * Will destroy all data
	 */
	void setDimension(qint32 rows, qint32 columns, ElementType initValue =
			ElementType()) {
		initMatrix(rows, columns, initValue);
	}

	qint32 getRowCount() const {
		return rows;
	}

	qint32 getColumnCount() const {
		return columns;
	}

	qint32 getLength() const {
		return rows * columns;
	}

	void setRowCount(qint32 rowCount) {
		initMatrix(rowCount, columns);
	}

	void setColumnCount(qint32 columnCount) {
		initMatrix(rows, columnCount);
	}

	/**
	 * Iterators
	 */
	Iterator begin() {
		return array.begin();
	}

	Iterator end() {
		return array.end();
	}

private:
	void initMatrix(qint32 rows, qint32 columns, ElementType initValue =
			ElementType()) {
		this->rows = rows;
		this->columns = columns;
		qint32 length = rows * columns;
		array.clear();
		for (qint32 i = 0; i < length; ++i) {
			array << initValue;
		}
	}

private:
	QList<ElementType> array;
	qint32 rows;
	qint32 columns;
};

typedef Matrix<double> RealMatrix;
typedef Matrix<QPoint> PointMatrix;

// Methods to save Patches
template<typename ElementType>
QDataStream & operator<<(QDataStream & stream,
		const Matrix<ElementType> & matrix);
template<typename ElementType>
QDataStream & operator>>(QDataStream & stream, Matrix<ElementType> & matrix);

template<typename ElementType>
QDataStream & operator<<(QDataStream & stream,
		const Matrix<ElementType> & matrix) {
	stream << matrix.getRowCount();
	stream << matrix.getColumnCount();
	for (qint32 r = 0; r < matrix.getRowCount(); ++r) {
		for (qint32 c = 0; c < matrix.getColumnCount(); ++c) {
			stream << matrix.getValue(r, c);
		}
	}
	return stream;
}

template<typename ElementType>
QDataStream & operator>>(QDataStream & stream, Matrix<ElementType> & matrix) {
	qint32 val;
	stream >> val;
	matrix.setRowCount(val);
	stream >> val;
	matrix.setColumnCount(val);
	ElementType elem;
	for (qint32 r = 0; r < matrix.getRowCount(); ++r) {
		for (qint32 c = 0; c < matrix.getColumnCount(); ++c) {
			stream >> elem;
			matrix.setValue(r, c, elem);
		}
	}
	return stream;
}

#endif /* MATRIX_H_ */

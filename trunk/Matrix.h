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
	Matrix(int rows = 0, int columns = 0) {
		initMatrix(rows, columns);
	}

	virtual ~Matrix() {
	}

	ElementType & getValue(int row, int column) {
		return array[row * columns + column];
	}

	const ElementType & getValue(int row, int column) const {
		return array[row * columns + column];
	}

	void setValue(int row, int column, const ElementType & value) {
		getValue(row, column) = value;
	}

	/**
	 * Will destroy all data
	 */
	void setDimension(int rows, int columns, ElementType initValue =
			ElementType()) {
		initMatrix(rows, columns, initValue);
	}

	int getRowCount() const {
		return rows;
	}

	int getColumnCount() const {
		return columns;
	}

	int getLength() const {
		return rows * columns;
	}

	void setRowCount(int rowCount) {
		initMatrix(rowCount, columns);
	}

	void setColumnCount(int columnCount) {
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
	void initMatrix(int rows, int columns, ElementType initValue =
			ElementType()) {
		this->rows = rows;
		this->columns = columns;
		int length = rows * columns;
		array.clear();
		for (int i = 0; i < length; ++i) {
			array << initValue;
		}
	}

private:
	QList<ElementType> array;
	int rows;
	int columns;
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
	for (int r = 0; r < matrix.getRowCount(); ++r) {
		for (int c = 0; c < matrix.getColumnCount(); ++c) {
			stream << matrix.getValue(r, c);
		}
	}
	return stream;
}

template<typename ElementType>
QDataStream & operator>>(QDataStream & stream, Matrix<ElementType> & matrix) {
	int val;
	stream >> val;
	matrix.setRowCount(val);
	stream >> val;
	matrix.setColumnCount(val);
	ElementType elem;
	for (int r = 0; r < matrix.getRowCount(); ++r) {
		for (int c = 0; c < matrix.getColumnCount(); ++c) {
			stream >> elem;
			matrix.setValue(r, c, elem);
		}
	}
	return stream;
}

#endif /* MATRIX_H_ */

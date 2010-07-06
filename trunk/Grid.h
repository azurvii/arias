/*
 * Grid.h
 *
 *  Created on: Jun 9, 2010
 *      Author: Vincent
 */

#ifndef GRID_H_
#define GRID_H_

#include <QDataStream>
#include <QList>
#include <QRect>
class Grid;

// Methods to save Grids
QDataStream & operator<<(QDataStream & stream, const Grid & grid);
QDataStream & operator>>(QDataStream & stream, Grid & grid);

class Grid {
public:
	enum GridType {
		GRID_NULL, GRID_ELLIPSE, GRID_RECTANGLE
	};

public:
	Grid(GridType type = GRID_NULL);
	virtual ~Grid();

public:
	GridType getType() const;
	QList<QRect> getGeometries() const;
	qint32 size() const;
	void clear();
	const QRect & operator[](qint32 i) const;
	QRect & operator[](qint32 i);
	void setType(GridType type);
	void setGeometries(const QList<QRect> & grid);
	void appendGeometry(const QRect & geometry);
	void remove(qint32 i);

private:
	GridType type;
	QList<QRect> grid;
};

#endif /* GRID_H_ */

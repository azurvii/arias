/*
 * Patch.h
 *
 *  Created on: Jun 9, 2010
 *      Author: Vincent
 */

#ifndef PATCH_H_
#define PATCH_H_

#include <QRect>
#include <QPolygon>
#include <QDataStream>
#include <QList>

class Patch;
typedef QList<Patch> PatchList;

// Methods to save Patches
QDataStream & operator<<(QDataStream & stream, const Patch & patch);
QDataStream & operator>>(QDataStream & stream, Patch & patch);
QDataStream & operator<<(QDataStream & stream, const PatchList & patchList);
QDataStream & operator>>(QDataStream & stream, PatchList & patchList);

class Patch {
public:
	enum PatchType {
		PATCH_NULL, PATCH_POLYGON, PATCH_ELLIPSE, PATCH_RECTANGLE
	};

public:
	Patch(PatchType type = PATCH_NULL);
	virtual ~Patch();

	void setType(PatchType type);
	PatchType getType() const;
	void setPolygon(const QPolygon & polygon);
	void setRectangle(const QRect & rectangle);
	const QPolygon & getPolygon() const;
	const QRect & getRectangle() const;
	const QRect & getEllipse() const;

private:
	PatchType type;
	QPolygon polygon;
	QRect rectangle;
};

typedef QList<Patch> PatchList;

#endif /* PATCH_H_ */

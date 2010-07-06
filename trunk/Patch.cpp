/*
 * Patch.cpp
 *
 *  Created on: Jun 9, 2010
 *      Author: Vincent
 */

#include "Patch.h"

QDataStream & operator<<(QDataStream & stream, const Patch & patch) {
	stream << (int) patch.getType();
	switch (patch.getType()) {
	case Patch::PATCH_NULL:
		break;
	case Patch::PATCH_ELLIPSE:
	case Patch::PATCH_RECTANGLE:
		stream << patch.getRectangle();
		break;
	case Patch::PATCH_POLYGON:
		stream << patch.getPolygon();
		break;
	}
	return stream;
}

QDataStream & operator>>(QDataStream & stream, Patch & patch) {
	int patchType;
	stream >> patchType;
	QRect rect;
	QPolygon polygon;
	switch ((Patch::PatchType) patchType) {
	case Patch::PATCH_NULL:
		break;
	case Patch::PATCH_ELLIPSE:
	case Patch::PATCH_RECTANGLE:
		stream >> rect;
		patch.setRectangle(rect);
		break;
	case Patch::PATCH_POLYGON:
		stream >> polygon;
		patch.setPolygon(polygon);
		break;
	}
	return stream;
}

QDataStream & operator<<(QDataStream & stream, const PatchList & patchList) {
	for (int i = 0; i < patchList.size(); ++i) {
		stream << patchList[i];
	}
	return stream;
}

QDataStream & operator>>(QDataStream & stream, PatchList & patchList) {
	patchList.clear();
	for (int i = 0; i < patchList.size(); ++i) {
		Patch patch;
		stream >> patch;
		patchList.append(patch);
	}
	return stream;
}

Patch::Patch(PatchType type) :
	type(type) {
}

Patch::~Patch() {
}

void Patch::setType(PatchType type) {
	this->type = type;
}

Patch::PatchType Patch::getType() const {
	return type;
}

void Patch::setPolygon(const QPolygon & polygon) {
	this->polygon = polygon;
}

void Patch::setRectangle(const QRect & rectangle) {
	this->rectangle = rectangle;
}

const QPolygon & Patch::getPolygon() const {
	return this->polygon;
}

const QRect & Patch::getRectangle() const {
	return this->rectangle;
}

const QRect & Patch::getEllipse() const {
	return getRectangle();
}

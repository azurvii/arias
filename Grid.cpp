/*
 * Grid.cpp
 *
 *  Created on: Jun 9, 2010
 *      Author: Vincent
 */

#include "Grid.h"

QDataStream & operator<<(QDataStream & stream, const Grid & grid) {
	stream << (int) grid.getType();
	stream << (int) grid.getGeometries().size();
	switch (grid.getType()) {
	case Grid::GRID_NULL:
		break;
	case Grid::GRID_ELLIPSE:
	case Grid::GRID_RECTANGLE:
		for (int i = 0; i < grid.getGeometries().size(); ++i) {
			stream << grid.getGeometries()[i];
		}
		break;
	}
	return stream;
}

QDataStream & operator>>(QDataStream & stream, Grid & grid) {
	int gridType, gsize;
	stream >> gridType;
	stream >> gsize;
	grid.setType((Grid::GridType) gridType);
	grid.clear();
	switch (gridType) {
	case Grid::GRID_NULL:
		break;
	case Grid::GRID_ELLIPSE:
	case Grid::GRID_RECTANGLE:
		for (int i = 0; i < gsize; ++i) {
			QRect geometry;
			stream >> geometry;
			grid.appendGeometry(geometry);
		}
		break;
	}
	return stream;
}

Grid::Grid(Grid::GridType type) {
	this->type = type;
}

Grid::~Grid() {
}

void Grid::setType(Grid::GridType type) {
	this->type = type;
}

Grid::GridType Grid::getType() const {
	return type;
}

void Grid::setGeometries(const QList<QRect> & grid) {
	this->grid = grid;
}

void Grid::appendGeometry(const QRect & geometry) {
	this->grid << geometry;
}

void Grid::clear() {
	grid.clear();
}

QList<QRect> Grid::getGeometries() const {
	return grid;
}

int Grid::size() const {
	return grid.size();
}

const QRect & Grid::operator[](int i) const {
	return grid[i];
}

QRect & Grid::operator[](int i) {
	return grid[i];
}

void Grid::remove(int i) {
	grid.removeAt(i);
}

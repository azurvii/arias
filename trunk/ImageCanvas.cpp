/*
 * ImageCanvas.cpp
 *
 *  Created on: Jun 3, 2010
 *      Author: Vincent
 */

#include "ImageCanvas.h"
#include "Processor.h"
#include <QtGui>

ImageCanvas::ImageCanvas(QWidget * parent) :
	QLabel(parent) {
	scale = 1.0;
	isGridShown = false;
	isPatchesShown = false;
	init();
}

ImageCanvas::~ImageCanvas() {
}

void ImageCanvas::setProcessor(Processor * processor) {
	this->processor = processor;
}

QBitmap ImageCanvas::getMask() const {
	if (image == 0) {
		return QBitmap();
	}
	QBitmap mask(image->size());
	mask.clear();
	QBrush brush;
	QPen pen;
	brush.setColor(QColor(Qt::color1));
	brush.setStyle(Qt::SolidPattern);
	pen.setStyle(Qt::NoPen);
	QPainter painter(&mask);
	painter.setPen(pen);
	painter.setBrush(brush);
	if (isGridShown && grid && grid->size() > 0) {
		painter.save();
		switch (grid->getType()) {
		case Grid::GRID_NULL:
			break;
		case Grid::GRID_ELLIPSE:
			for (int i = 0; i < grid->size(); ++i) {
				painter.drawEllipse((*grid)[i]);
			}
			break;
		case Grid::GRID_RECTANGLE:
			for (int i = 0; i < grid->size(); ++i) {
				painter.drawRect((*grid)[i]);
			}
			break;
		}
		painter.restore();
	}
	if (isPatchesShown && patches && patches->size() > 0) {
		painter.save();
		brush.setColor(QColor(Qt::color0));
		painter.setBrush(brush);
		for (int i = 0; i < patches->size(); ++i) {
			switch ((*patches)[i].getType()) {
			case Patch::PATCH_ELLIPSE:
				painter.drawEllipse((*patches)[i].getEllipse());
				break;
			case Patch::PATCH_POLYGON:
				painter.drawPolygon((*patches)[i].getPolygon());
				break;
			case Patch::PATCH_RECTANGLE:
				painter.drawRect((*patches)[i].getRectangle());
				break;
			case Patch::PATCH_NULL:
				break;
			}
		}
		painter.restore();
	}
	painter.end();
	return mask;
}

void ImageCanvas::paintEvent(QPaintEvent *event) {
	QLabel::paintEvent(event);
	QPainter painter(this);
	painter.scale(scale, scale);
	// for drawing the grid
	if (isGridShown && grid && grid->size() > 0) {
		painter.save();
		switch (grid->getType()) {
		case Grid::GRID_NULL:
			break;
		case Grid::GRID_ELLIPSE:
			for (int i = 0; i < grid->size(); ++i) {
				painter.drawEllipse((*grid)[i]);
			}
			break;
		case Grid::GRID_RECTANGLE:
			for (int i = 0; i < grid->size(); ++i) {
				painter.drawRect((*grid)[i]);
			}
			break;
		}
		painter.restore();
	}
	// for drawing the patches
	if (isPatchesShown && patches && patches->size() > 0) {
		painter.save();
		QBrush brush;
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(QColor(Qt::color1));
		for (int i = 0; i < patches->size(); ++i) {
			switch ((*patches)[i].getType()) {
			case Patch::PATCH_ELLIPSE:
				painter.drawEllipse((*patches)[i].getEllipse());
				break;
			case Patch::PATCH_POLYGON:
				painter.drawPolygon((*patches)[i].getPolygon());
				break;
			case Patch::PATCH_RECTANGLE:
				painter.drawRect((*patches)[i].getRectangle());
				break;
			case Patch::PATCH_NULL:
				break;
			}
		}
		painter.restore();
	}
}

void ImageCanvas::showGrid(bool show) {
	isGridShown = show;
	update();
}

void ImageCanvas::hideGrid(bool hide) {
	showGrid(!hide);
}

void ImageCanvas::showPatches(bool show) {
	isPatchesShown = show;
	update();
}

void ImageCanvas::hidePatches(bool hide) {
	showPatches(!hide);
}

void ImageCanvas::init() {
	grid = 0;
	patches = 0;
	image = 0;
	imageInverted = false;
}

void ImageCanvas::setScale(double imageScale) {
	if (scale != imageScale) {
		scale = imageScale;
		emit imageScaleChanged(scale);
	}
	updateImage();
}

void ImageCanvas::setGrid(const Grid * grid) {
	if (this->grid != grid) {
		this->grid = grid;
	}
	updateGrid();
}

void ImageCanvas::updateGrid() {
	update();
}

void ImageCanvas::setPatches(const PatchList * patches) {
	if (this->patches != patches) {
		this->patches = patches;
	}
	updatePatches();
}

void ImageCanvas::updatePatches() {
	update();
}

double ImageCanvas::getScale() const {
	return scale;
}

void ImageCanvas::setImage(const QImage * image) {
	if (this->image != image) {
		this->image = image;
	}
	updateImage();
}

void ImageCanvas::updateImage() {
	scaleImage();
	update();
}

void ImageCanvas::scaleImage() {
	if (image) {
		QImage dispImage = *image;
		if (imageInverted) {
			dispImage.invertPixels();
		}
		this->setPixmap(QPixmap::fromImage(dispImage).scaled(dispImage.width()
				* scale, dispImage.height() * scale));
	}
}

void ImageCanvas::setImageInverted(bool inverted) {
	imageInverted = inverted;
	scaleImage();
	update();
}

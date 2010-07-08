/*
 * ImageCanvas.h
 *
 *  Created on: Jun 3, 2010
 *      Author: Vincent
 */

#ifndef IMAGECANVAS_H_
#define IMAGECANVAS_H_

#include <QLabel>
#include "Grid.h"
#include "Patch.h"

class Processor;
class QImage;

class ImageCanvas: public QLabel {
Q_OBJECT
public:
	ImageCanvas(QWidget * parent = 0);
	virtual ~ImageCanvas();

public:
	QBitmap getMask() const;
	double getScale() const;

public slots:
	void showGrid(bool show = true);
	void hideGrid(bool hide = true);
	void showPatches(bool show = true);
	void hidePatches(bool hide = true);
	void setScale(double imageScale);
	void setImage(const QImage * image);
	void updateImage();
	void setProcessor(Processor * processor);
	void setGrid(const Grid * grid);
	void updateGrid();
	void setPatches(const PatchList * patches);
	void updatePatches();
	void setImageInverted(bool inverted);

signals:
	void imageScaleChanged(double imageScale);

protected:
	void paintEvent(QPaintEvent *event);

private:
	Processor *processor;
	bool isGridShown;
	bool isPatchesShown;
	double scale;
	const QImage * image;
	const Grid *grid;
	const PatchList *patches;
	bool imageInverted;

private:
	void init();
	void scaleImage();
};

#endif /* IMAGECANVAS_H_ */

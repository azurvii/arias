/*
 * Processor.h
 *
 *  Created on: May 27, 2010
 *      Author: Vincent
 */

#ifndef PROCESSOR_H_
#define PROCESSOR_H_

#include <QObject>
#include <QImage>
#include <QList>
#include <QPoint>
#include "Matrix.h"
#include "Patch.h"
#include "Grid.h"

class Processor: public QObject {
Q_OBJECT
public:
	enum MatrixType {
		MATRIX_GRAY_VALUE = 0,
		MATRIX_MASKED_GRAY_VALUE = 1,
		MATRIX_AVERAGES = 2,
		MATRIX_COUNTS = 3, // should contains ints only, defined as double for simplicity
		MATRIX_TOTAL = 4
	};
	enum HeaderType {
		HEADER_ROW = 0,
		HEADER_COL = 1,
		HEADER_TOTAL = 2
	};

public:
	Processor(QObject * parent = 0);
	virtual ~Processor();
	const QImage & getImage() const;
	const Grid & getGrid() const;
	const PatchList & getPatches() const;
	const RealMatrix & getMatrix(MatrixType matrixType) const;
	int getGridRowCount() const;
	int getGridColumnCount() const;
	double getGridAngle() const;
	double getGridElementWidth() const;
	double getGridElementHeight() const;
	QPoint getGridStartPoint() const;
	QPoint getGridEndPoint() const;
	const QStringList & getRowHeaders() const;
	const QStringList & getImageRowHeaders() const;
	const QStringList & getColumnHeaders() const;
	const QStringList & getImageColumnHeaders() const;
	qint64 getGrayMax() const;
	qint64 getGrayMin() const;
	qint64 getUpperBorder() const;
	qint64 getLowerBorder() const;
	bool isReady(MatrixType type) const;
	QList<double> getAverageRow(int row) const;
	QList<double> getAverageColumn(int column) const;
//	const QList<double> & getMadRanges() const;

public slots:
	bool loadProject(const QString & projectFilePath);
	void saveProject(const QString & projectFilePath);
	void saveAverageIntensities(const QString & filePath);
	void loadImage(const QString & imageFilePath, bool inverted = true);
	void setRowHeaders(const QStringList & rowHeaders);
	void setColHeaders(const QStringList & colHeaders);
	void setMask(const QBitmap &mask);
	void clearPatches();
	void appendPatch(const Patch & patch);
	void clearLastPatch();
	void setGridRowCount(int rows);
	void setGridColumnCount(int cols);
	void setGridDimension(int rows, int cols);
	void setGridAngle(double angle);
	void setGridStartPoint(const QPoint & startPoint);
	void setGridStartPointX(double x);
	void setGridStartPointY(double y);
	void setGridEndPoint(const QPoint & endPoint);
	void setGridEndPointX(double x);
	void setGridEndPointY(double y);
	void setGridElementWidth(double width);
	void setGridElementHeight(double height);
	void setGridType(Grid::GridType type);
	void setGridOrtho(bool isOrtho);
	void invertImage();
//	void omitRow(int row);
//	void omitColumn(int column);
	void calculateAverages();
//	void setMadRanges(const QList<double> & madRanges);
	void setMatrixValue(MatrixType type, int row, int column, double value);

signals:
	void imageChanged(const QImage * image);
	void gridChanged(const Grid * grid);
	void patchListChanged(const PatchList * patches);
	void gridAngleChanged(double angle);
	void gridRowCountChanged(int rows);
	void gridColumnCountChanged(int cols);
	void gridOverlapped(bool overlapped);
	void gridElementWidthChanged(double elementWidth);
	void gridElementHeightChanged(double elementHeight);
	void gridStartXChanged(double x);
	void gridStartYChanged(double y);
	void gridEndXChanged(double x);
	void gridEndYChanged(double y);
	void matrixChanged(int type, RealMatrix * matrix);
	void message(const QString & message);

private:
	void calculateGrid();
	void scanImage();
	void debug(const QString &message);
	bool isImageValid();

private:
	static const qint32 projectFileHeader;
	static const qint32 projectFileVersion;
	static const QString defaultRowHeaderName;
	static const QString defaultColHeaderName;
	QImage image;
	QList<RealMatrix> matrices;
	QList<bool> ready;
	qint64 grayMax, grayMin;
	qint64 borderLower, borderUpper;
	QList<QStringList> headers;
	Grid grid;
	PatchList patches;
	double gridAngle;
	double gridElementWidth;
	double gridElementHeight;
	QPoint gridStartPoint;
	QPoint gridEndPoint;
	bool isOrthoGrid;
};

#endif /* PROCESSOR_H_ */

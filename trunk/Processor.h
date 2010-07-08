/*
 * Processor.h
 *
 *  Created on: May 27, 2010
 *      Author: Vincent
 */

#ifndef PROCESSOR_H_
#define PROCESSOR_H_

#include <QImage>
#include <QString>
#include <QList>
#include "Matrix.h"
#include "Patch.h"
#include "Grid.h"

class Processor: public QObject {
Q_OBJECT
public:
	enum MatrixType {
		MATRIX_VALUE = 0,
		MATRIX_MASKED = 1,
		MATRIX_AVERAGE = 2,
		MATRIX_COUNT = 3, // should contains ints only, defined as double for simplicity
		MATRIX_TOTAL = 4
	};

	enum ChannelType {
		CHANNEL_GRAY = 0,
		CHANNEL_RED = 1,
		CHANNEL_GREEN = 2,
		CHANNEL_BLUE = 3,
		CHANNEL_TOTAL = 4,
	};

	enum HeaderType {
		HEADER_ROW = 0, HEADER_COL = 1, HEADER_TOTAL = 2
	};

public:
	Processor(QObject * parent = 0);
	virtual ~Processor();
	const QImage & getImage() const;
	const Grid & getGrid() const;
	const PatchList & getPatches() const;
	const RealMatrix & getMatrix(MatrixType matrixType) const;
	qint32 getGridRowCount() const;
	qint32 getGridColumnCount() const;
	double getGridAngle() const;
	double getGridElementWidth() const;
	double getGridElementHeight() const;
	QPoint getGridStartPoint() const;
	QPoint getGridEndPoint() const;
	const QStringList & getRowHeaders() const;
	const QStringList & getImageRowHeaders() const;
	const QStringList & getColumnHeaders() const;
	const QStringList & getImageColumnHeaders() const;
	qint64 getMaxColor() const;
	qint64 getMinColor() const;
	qint64 getUpperBorder() const;
	qint64 getLowerBorder() const;
	bool isReady(MatrixType type) const;
	QList<double> getAverageRow(qint32 row) const;
	QList<double> getAverageColumn(qint32 column) const;
	ChannelType getChannel() const;
	bool isWhiteBackground() const;

public slots:
	bool loadProject(const QString & projectFilePath);
	void saveProject(const QString & projectFilePath);
	void saveAverageIntensities(const QString & filePath);
	void loadImage(const QString & imageFilePath, bool inverted = false);
	void setRowHeaders(const QStringList & rowHeaders);
	void setColHeaders(const QStringList & colHeaders);
	void setMask(const QBitmap &mask);
	void clearPatches();
	void appendPatch(const Patch & patch);
	void clearLastPatch();
	void setGridRowCount(int rows);
	void setGridColumnCount(int cols);
	void setGridDimension(qint32 rows, qint32 cols);
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
	void setWhiteBackground(bool white);
	void invertImage();
	void calculateAverages();
	void setMatrixValue(MatrixType type, qint32 row, qint32 column,
			double value);
	void setChannel(ChannelType channel);

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
	void matrixChanged(qint32 type, RealMatrix * matrix);
	void message(const QString & message);

private:
	void calculateGrid();
	void scanImage();
	void debug(const QString &message);
	bool isImageValid();
	qint64 getCurrentChannelColor(const QRgb & rgb);

private:
	static const qint32 projectFileHeader;
	static const qint32 projectFileVersion;
	static const QString defaultRowHeaderName;
	static const QString defaultColHeaderName;
	QImage image;
	QList<RealMatrix> matrices;
	QList<bool> ready;
	qint64 colorMax, colorMin;
	qint64 borderUpper, borderLower;
	QList<QStringList> headers;
	Grid grid;
	PatchList patches;
	double gridAngle;
	double gridElementWidth;
	double gridElementHeight;
	QPoint gridStartPoint;
	QPoint gridEndPoint;
	bool isOrthoGrid;
	ChannelType channel;
	bool whiteBackground;
};

#endif /* PROCESSOR_H_ */

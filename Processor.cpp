/*
 * Processor.cpp
 *
 *  Created on: May 27, 2010
 *      Author: Vincent
 */

#include "Processor.h"
#include <QtGui>
#include "stat.h"
#include <iostream>

const qint32 Processor::projectFileHeader = 0x2c48a8b9;
const qint32 Processor::projectFileVersion = 0x00000002;
const QString Processor::defaultRowHeaderName = QObject::tr("R");
const QString Processor::defaultColHeaderName = QObject::tr("C");

Processor::Processor(QObject * parent) :
	QObject(parent) {
	colorMax = -1;
	colorMin = -1;
	borderLower = -1;
	borderUpper = -1;
	gridAngle = .0;
	whiteBackground = true;
	gridElementWidth = gridElementHeight = .0;
	channel = CHANNEL_GRAY;
	for (qint32 i = 0; i < MATRIX_TOTAL; ++i) {
		matrices << RealMatrix();
		ready << false;
	}
	for (qint32 i = 0; i < HEADER_TOTAL; ++i) {
		headers << QStringList();
	}
}

Processor::~Processor() {
}

bool Processor::loadProject(const QString & projectFilePath) {
	QFile file(projectFilePath);
	if (file.open(QFile::ReadOnly) == false) {
		emit message(tr("!! Cannot open file %1.").arg(projectFilePath));
		return false;
	}
	QDataStream ds(&file);
	qint32 header, version;
	ds >> header >> version;
	if (header != projectFileHeader) {
		emit message(tr("!! This is not a YIMP project file."));
		return false;
	}
	if (version == projectFileVersion) {
		QByteArray buffer;
		ds >> buffer;
		buffer = qUncompress(buffer);
		QDataStream bds(&buffer, QIODevice::ReadOnly);
		qint32 mtotal;
		bds >> mtotal;
		if (mtotal != MATRIX_TOTAL) {
			emit message(tr("This is a damaged YIMP project file."));
			return false;
		}
		RealMatrix rm;
		bool rdy;
		matrices.clear();
		ready.clear();
		for (qint32 i = 0; i < MATRIX_TOTAL; ++i) {
			bds >> rm;
			bds >> rdy;
			matrices << rm;
			ready << rdy;
		}
		qint32 htotal;
		bds >> htotal;
		if (htotal != HEADER_TOTAL) {
			emit message(tr("This is a damaged YIMP project file."));
			return false;
		}
		QStringList h;
		headers.clear();
		for (qint32 i = 0; i < HEADER_TOTAL; ++i) {
			bds >> h;
			headers << h;
		}
		bds >> grid;
		bds >> patches;
		bds >> gridAngle;
		bds >> gridElementWidth;
		bds >> gridElementHeight;
		qint32 x, y;
		bds >> x;
		bds >> y;
		gridStartPoint = QPoint(x, y);
		bds >> x;
		bds >> y;
		gridEndPoint = QPoint(x, y);
		bds >> image;
		qint32 value;
		bds >> value;
		channel = (ChannelType) value;
		if (!isImageValid()) {
			return false;
		}
		scanImage();
		emit imageChanged(&image);
		return true;
	} else {
		emit message(tr("!! This YIMP project version is not supported."));
		return false;
	}
}

void Processor::saveProject(const QString & projectFilePath) {
	QFile file(projectFilePath);
	if (file.open(QFile::WriteOnly) == false) {
		emit message(tr("!! Cannot open file %1.").arg(projectFilePath));
		return;
	}
	QDataStream ds(&file);
	ds << projectFileHeader;
	ds << projectFileVersion;
	QByteArray buffer;
	QDataStream bds(&buffer, QIODevice::WriteOnly);
	bds << (qint32) MATRIX_TOTAL;
	for (qint32 i = 0; i < MATRIX_TOTAL; ++i) {
		bds << matrices[i];
		bds << ready[i];
	}
	bds << (qint32) HEADER_TOTAL;
	for (qint32 i = 0; i < HEADER_TOTAL; ++i) {
		bds << headers[i];
	}
	bds << grid;
	bds << patches;
	bds << gridAngle;
	bds << gridElementWidth;
	bds << gridElementHeight;
	bds << (qint32) gridStartPoint.x();
	bds << (qint32) gridStartPoint.y();
	bds << (qint32) gridEndPoint.x();
	bds << (qint32) gridEndPoint.y();
	bds << image;
	bds << (qint32) channel;
	ds << qCompress(buffer);
}

void Processor::loadImage(const QString & imageFilePath, bool inverted) {
	image = QImage();
	if (image.load(imageFilePath)) {
		if (!isImageValid()) {
			return;
		}
		//		if (inverted) {
		//			image.invertPixels();
		//		}
		scanImage();
		emit imageChanged(&image);
	} else {
		emit message(tr("!! Failed loading image: \"%1\"").arg(imageFilePath));
	}
}

void Processor::scanImage() {
	qint32 rows = image.height();
	qint32 cols = image.width();
	RealMatrix * valueMatrix = &matrices[MATRIX_VALUE];
	RealMatrix * maskedMatrix = &matrices[MATRIX_MASKED];
	valueMatrix->setDimension(rows, cols);
	maskedMatrix->setDimension(rows, cols, -1.0);
	QImage theImageToBe = image;
	if (whiteBackground) {
		theImageToBe.invertPixels();
	}
	colorMax = colorMin = getCurrentChannelColor(theImageToBe.pixel(0, 0));
	for (qint32 r = 0; r < rows; ++r) {
		for (qint32 c = 0; c < cols; ++c) {
			qint32 value = getCurrentChannelColor(theImageToBe.pixel(c, r));
			valueMatrix->setValue(r, c, value);
			maskedMatrix->setValue(r, c, value);
			colorMax = colorMax > value ? colorMax : value;
			colorMin = colorMin < value ? colorMin : value;
		}
	}
	ready[MATRIX_VALUE] = true;
	ready[MATRIX_MASKED] = true;
	emit matrixChanged(MATRIX_VALUE, valueMatrix);
	emit matrixChanged(MATRIX_MASKED, maskedMatrix);
}

void Processor::setRowHeaders(const QStringList & rowHeaders) {
	setGridRowCount(rowHeaders.size());
	headers[HEADER_ROW] = rowHeaders;
}

void Processor::setColHeaders(const QStringList & colHeaders) {
	setGridColumnCount(colHeaders.size());
	headers[HEADER_COL] = colHeaders;
}

const RealMatrix & Processor::getMatrix(MatrixType type) const {
	return matrices[type];
}

const QImage & Processor::getImage() const {
	return image;
}

void Processor::setMask(const QBitmap &mask) {
	qint32 w = image.width();
	qint32 h = image.height();
	QImage maskImage = mask.toImage();
	if (!(w == maskImage.width() && h == maskImage.height())) {
		return;
	}
	QRgb color1Rgb = QColor(Qt::color1).rgb();
	RealMatrix & maskedMatrix = matrices[MATRIX_MASKED];
	RealMatrix & valueMatrix = matrices[MATRIX_VALUE];
	for (qint32 r = 0; r < h; ++r) {
		for (qint32 c = 0; c < w; ++c) {
			if (maskImage.pixel(c, r) != color1Rgb) {
				maskedMatrix.setValue(r, c, -1.0);
			} else {
				maskedMatrix.setValue(r, c, valueMatrix.getValue(r, c));
			}
		}
	}
	ready[MATRIX_MASKED] = true;
	emit Processor::matrixChanged(MATRIX_MASKED, &maskedMatrix);
}

void Processor::appendPatch(const Patch & patch) {
	patches << patch;
	emit patchListChanged(&patches);
}

void Processor::clearLastPatch() {
	if (patches.size() > 0) {
		patches.pop_back();
		emit patchListChanged(&patches);
	}
}

void Processor::setGridRowCount(int rows) {
	qint32 curRows = headers[HEADER_ROW].size();
	if (rows == curRows) {
		return;
	}
	if (curRows > rows) {
		for (qint32 r = 0; r < curRows - rows; ++r) {
			headers[HEADER_ROW].pop_back();
		}
	} else { // curRows < rows
		for (qint32 r = 0; r < rows - curRows; ++r) {
			headers[HEADER_ROW].push_back(defaultRowHeaderName);
		}
	}
	calculateGrid();
	matrices[MATRIX_AVERAGE].setRowCount(rows);
	matrices[MATRIX_COUNT].setRowCount(rows);
	ready[MATRIX_AVERAGE] = false;
	ready[MATRIX_COUNT] = false;
	emit gridRowCountChanged(rows);
}

void Processor::setGridColumnCount(int cols) {
	qint32 curCols = headers[HEADER_COL].size();
	if (cols == curCols) {
		return;
	}
	if (curCols > cols) {
		for (qint32 c = 0; c < curCols - cols; ++c) {
			headers[HEADER_COL].pop_back();
		}
	} else { // curRows < rows
		for (qint32 c = 0; c < cols - curCols; ++c) {
			headers[HEADER_COL].push_back(defaultColHeaderName);
		}
	}
	calculateGrid();
	matrices[MATRIX_AVERAGE].setColumnCount(cols);
	matrices[MATRIX_COUNT].setColumnCount(cols);
	ready[MATRIX_AVERAGE] = false;
	ready[MATRIX_COUNT] = false;
	emit gridColumnCountChanged(cols);
}

const Grid & Processor::getGrid() const {
	return grid;
}

const PatchList & Processor::getPatches() const {
	return patches;
}

qint32 Processor::getGridRowCount() const {
	return headers[HEADER_ROW].size();
}

qint32 Processor::getGridColumnCount() const {
	return headers[HEADER_COL].size();
}

void Processor::calculateGrid() {
	QTransform trsf;
	grid.clear();
	trsf.translate(gridStartPoint.x(), gridStartPoint.y());
	trsf.rotate(gridAngle);
	double intervX = getGridColumnCount() == 1 ? 0 : double(gridEndPoint.x()
			- gridStartPoint.x()) / double(getGridColumnCount() - 1);
	double intervY = getGridRowCount() == 1 ? 0 : double(gridEndPoint.y()
			- gridStartPoint.y()) / double(getGridRowCount() - 1);
	double w = gridElementWidth;
	double h = isOrthoGrid ? gridElementWidth : gridElementHeight;
	double x = -w / 2.0;
	double y = -h / 2.0;
	if (w > intervX || h > intervY) {
		emit gridOverlapped(true);
	} else {
		emit gridOverlapped(false);
	}
	for (qint32 r = 0; r < headers[HEADER_ROW].size(); ++r) {
		for (qint32 c = 0; c < headers[HEADER_COL].size(); ++c) {
			switch (grid.getType()) {
			case Grid::GRID_NULL:
				break;
			case Grid::GRID_ELLIPSE:
			case Grid::GRID_RECTANGLE:
				grid.appendGeometry(trsf.mapRect(QRect(x + c * intervX, y + r
						* intervY, w, h)));
				break;
			}
		}
	}
	//	calculateAverages();
	emit gridChanged(&grid);
}

void Processor::setGridDimension(qint32 rows, qint32 cols) {
	setGridRowCount(rows);
	setGridColumnCount(cols);
}

void Processor::setGridAngle(double angle) {
	if (gridAngle == angle) {
		return;
	}
	gridAngle = angle;
	emit gridAngleChanged(angle);
	calculateGrid();
}

void Processor::setGridStartPoint(const QPoint & startPoint) {
	if (gridStartPoint == startPoint) {
		return;
	}
	gridStartPoint = startPoint;
	emit gridStartXChanged(startPoint.x());
	emit gridStartYChanged(startPoint.y());
	calculateGrid();
}

void Processor::setGridEndPoint(const QPoint & endPoint) {
	if (gridEndPoint == endPoint) {
		return;
	}
	gridEndPoint = endPoint;
	emit gridEndXChanged(endPoint.x());
	emit gridEndYChanged(endPoint.y());
	calculateGrid();
}

void Processor::setGridElementWidth(double width) {
	if (gridElementWidth == width) {
		return;
	}
	gridElementWidth = width;
	emit gridElementWidthChanged(width);
	calculateGrid();
}

void Processor::setGridElementHeight(double height) {
	if (gridElementHeight == height) {
		return;
	}
	gridElementHeight = height;
	emit gridElementHeightChanged(height);
	calculateGrid();
}

void Processor::setGridType(Grid::GridType type) {
	grid.setType(type);
}

void Processor::setGridOrtho(bool isOrtho) {
	isOrthoGrid = isOrtho;
	calculateGrid();
}

double Processor::getGridElementWidth() const {
	return gridElementWidth;
}

double Processor::getGridElementHeight() const {
	return gridElementHeight;
}

QPoint Processor::getGridStartPoint() const {
	return gridStartPoint;
}

QPoint Processor::getGridEndPoint() const {
	return gridEndPoint;
}

const QStringList & Processor::getRowHeaders() const {
	return headers[HEADER_ROW];
}

const QStringList & Processor::getColumnHeaders() const {
	return headers[HEADER_COL];
}

void Processor::clearPatches() {
	patches.clear();
	emit patchListChanged(&patches);
}

void Processor::invertImage() {
	image.invertPixels();
	scanImage();
	emit imageChanged(&image);
}

void Processor::calculateAverages() {
	if (image.isNull() || grid.getType() == Grid::GRID_NULL
			|| grid.getGeometries().size() == 0 || ready[MATRIX_MASKED]
			== false) {
		return;
	}
	RealMatrix & averages = matrices[MATRIX_AVERAGE];
	RealMatrix & counts = matrices[MATRIX_COUNT];
	RealMatrix & masked = matrices[MATRIX_MASKED];
	double value;
	for (qint32 i = 0; i < grid.size(); ++i) {
		double sum = .0;
		qint32 count = 0;
		QRect rect = grid[i];
		for (qint32 x = rect.left(); x < rect.right(); ++x) {
			if (x < 0 || x >= image.width()) {
				continue;
			}
			for (qint32 y = rect.top(); y < rect.bottom(); ++y) {
				if (y < 0 || y >= image.height()) {
					continue;
				}
				value = masked.getValue(y, x);
				if (value >= 0) {
					sum += value;
					++count;
				}
			}
		}
		qint32 r = i / headers[HEADER_COL].size();
		qint32 c = i % headers[HEADER_COL].size();
		averages.setValue(r, c, sum / count);
		counts.setValue(r, c, count);
	}
	ready[MATRIX_AVERAGE] = true;
	ready[MATRIX_COUNT] = true;
}

qint64 Processor::getMaxColor() const {
	return colorMax;
}

qint64 Processor::getMinColor() const {
	return colorMin;
}

bool Processor::isReady(MatrixType type) const {
	return ready[type];
}

QList<double> Processor::getAverageRow(qint32 row) const {
	QList<double> returnList;
	if (ready[MATRIX_AVERAGE] == false || row < 0 || row
			>= headers[HEADER_ROW].size()) {
		return returnList;
	}
	for (qint32 c = 0; c < matrices[MATRIX_AVERAGE].getColumnCount(); ++c) {
		returnList << matrices[MATRIX_AVERAGE].getValue(row, c);
	}
	return returnList;
}

QList<double> Processor::getAverageColumn(qint32 column) const {
	QList<double> returnList;
	if (ready[MATRIX_AVERAGE] == false || column < 0 || column
			>= headers[HEADER_COL].size()) {
		return returnList;
	}
	for (qint32 r = 0; r < matrices[MATRIX_AVERAGE].getRowCount(); ++r) {
		returnList << matrices[MATRIX_AVERAGE].getValue(r, column);
	}
	return returnList;
}

const QStringList & Processor::getImageRowHeaders() const {
	return headers[HEADER_ROW];
}

const QStringList & Processor::getImageColumnHeaders() const {
	return headers[HEADER_COL];
}

double Processor::getGridAngle() const {
	return gridAngle;
}

void Processor::setGridStartPointX(double x) {
	gridStartPoint.setX(x);
	emit gridStartXChanged(x);
	calculateGrid();
}

void Processor::setGridStartPointY(double y) {
	gridStartPoint.setY(y);
	emit gridStartYChanged(y);
	calculateGrid();
}

void Processor::setGridEndPointX(double x) {
	gridEndPoint.setX(x);
	emit gridEndXChanged(x);
	calculateGrid();
}

void Processor::setGridEndPointY(double y) {
	gridEndPoint.setY(y);
	emit gridEndYChanged(y);
	calculateGrid();
}

void Processor::debug(const QString &message) {
	std::cout << "DEBUG: " << message.toStdString() << std::endl;
}

void Processor::setMatrixValue(MatrixType type, qint32 row, qint32 column,
		double value) {
	RealMatrix & matrix = matrices[type];
	matrix.setValue(row, column, value);
}

qint64 Processor::getUpperBorder() const {
	return borderUpper;
}

qint64 Processor::getLowerBorder() const {
	return borderLower;
}

void Processor::saveAverageIntensities(const QString & filePath) {
	if (!isReady(MATRIX_AVERAGE)) {
		emit message(tr("!! Error: Average intensities not calculated yet!"));
		return;
	}
	QFile file(filePath);
	if (file.open(QFile::WriteOnly) == false) {
		emit message(tr("!! Cannot open file %1.").arg(filePath));
		return;
	}
	QTextStream ds(&file);
	QString hheader = getImageColumnHeaders().join("\t");
	hheader = "\t" + hheader;
	ds << hheader << "\n";
	const RealMatrix & mat = matrices[MATRIX_AVERAGE];
	qint32 rows = mat.getRowCount();
	for (qint32 r = 0; r < rows; ++r) {
		QString line = getImageRowHeaders()[r] + "\t";
		qint32 cols = mat.getColumnCount();
		for (qint32 c = 0; c < cols; ++c) {
			line += QString::number(mat.getValue(r, c));
			if (c < cols - 1) {
				line += "\t";
			}
		}
		ds << line << "\n";
	}
	emit message(tr("** Exported average intensities to \"%1\"").arg(filePath));
}

bool Processor::isImageValid() {
	switch (image.format()) {
	case QImage::Format_Mono:
		borderLower = 0;
		borderUpper = 1;
		break;
	case QImage::Format_RGB16:
		borderLower = 0;
		borderUpper = 63;
		break;
	case QImage::Format_Indexed8:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32:
		// they are all 8-bit gray image equivalents
		borderLower = 0;
		borderUpper = 255;
		break;
	default:
		borderLower = -1;
		borderUpper = -1;
		emit message(tr("!! Unsupported image format"));
		return false;
	}
	return true;
}

qint64 Processor::getCurrentChannelColor(const QRgb & rgb) {
	qint64 color;
	switch (channel) {
	case CHANNEL_GRAY:
		color = qGray(rgb);
		break;
	case CHANNEL_RED:
		color = qRed(rgb);
		break;
	case CHANNEL_GREEN:
		color = qGreen(rgb);
		break;
	case CHANNEL_BLUE:
		color = qBlue(rgb);
		break;
	default:
		color = -1;
		break;
	}
	return color;
}

Processor::ChannelType Processor::getChannel() const {
	return channel;
}

void Processor::setChannel(ChannelType channel) {
	this->channel = channel;
	scanImage();
}

bool Processor::isWhiteBackground() const {
	return whiteBackground;
}

void Processor::setWhiteBackground(bool white) {
	whiteBackground = white;
	scanImage();
}

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
const qint32 Processor::projectFileVersion = 0x00000001;
const QString Processor::defaultRowHeaderName = QObject::tr("R");
const QString Processor::defaultColHeaderName = QObject::tr("C");

Processor::Processor(QObject * parent) :
	QObject(parent) {
	grayMax = -1;
	grayMin = -1;
	borderLower = -1;
	borderUpper = -1;
	gridAngle = .0;
	gridElementWidth = gridElementHeight = .0;
	for (int i = 0; i < MATRIX_TOTAL; ++i) {
		matrices << RealMatrix();
		ready << false;
	}
	for (int i = 0; i < HEADER_TOTAL; ++i) {
		headers << QStringList();
//		omittedHeaders << QStringList();
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
		int mtotal;
		ds >> mtotal;
		if (mtotal != MATRIX_TOTAL) {
			emit message(tr("This is a damaged YIMP project file."));
			return false;
		}
		RealMatrix rm;
		bool rdy;
		matrices.clear();
		ready.clear();
		for (int i = 0; i < MATRIX_TOTAL; ++i) {
			ds >> rm;
			ds >> rdy;
			matrices << rm;
			ready << rdy;
		}
		int htotal;
		ds >> htotal;
		if (htotal != HEADER_TOTAL) {
			emit message(tr("This is a damaged YIMP project file."));
			return false;
		}
		QStringList h;
		headers.clear();
//		omittedHeaders.clear();
		for (int i = 0; i < HEADER_TOTAL; ++i) {
			ds >> h;
//			ds >> oh;
			headers << h;
//			omittedHeaders << oh;
		}
		ds >> grid;
		ds >> patches;
		ds >> gridAngle;
		ds >> gridElementWidth;
		ds >> gridElementHeight;
		int x, y;
		ds >> x;
		ds >> y;
		gridStartPoint = QPoint(x, y);
		ds >> x;
		ds >> y;
		gridEndPoint = QPoint(x, y);
		//		int val;
		//		int psize;
		//		ds >> psize;
		//		omittedRows.clear();
		//		for (int i = 0; i < psize; ++i) {
		//			ds >> val;
		//			omittedRows << val;
		//		}
		//		ds >> psize;
		//		omittedColumns.clear();
		//		for (int i = 0; i < psize; ++i) {
		//			ds >> val;
		//			omittedColumns << val;
		//		}
		//		ds >> psize;
		//		phosphoColumns.clear();
		//		for (int i = 0; i < psize; ++i) {
		//			ds >> val;
		//			phosphoColumns << val;
		//		}
		//		ds >> psize;
		//		double doubleVal;
		//		madRanges.clear();
		//		for (int i = 0; i < psize; ++i) {
		//			ds >> doubleVal;
		//			madRanges << doubleVal;
		//		}
		ds >> image;
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
	ds << (int) MATRIX_TOTAL;
	for (int i = 0; i < MATRIX_TOTAL; ++i) {
		ds << matrices[i];
		ds << ready[i];
	}
	ds << (int) HEADER_TOTAL;
	for (int i = 0; i < HEADER_TOTAL; ++i) {
		ds << headers[i];
		//		ds << omittedHeaders[i];
	}
	ds << grid;
	ds << patches;
	ds << gridAngle;
	ds << gridElementWidth;
	ds << gridElementHeight;
	ds << (int) gridStartPoint.x();
	ds << (int) gridStartPoint.y();
	ds << (int) gridEndPoint.x();
	ds << (int) gridEndPoint.y();
	//	ds << (int) omittedRows.size();
	//	for (int i = 0; i < omittedRows.size(); ++i) {
	//		ds << omittedRows[i];
	//	}
	//	int psize;
	//	psize = (int) omittedColumns.size();
	//	ds << psize;
	//	for (int i = 0; i < psize; ++i) {
	//		ds << omittedColumns[i];
	//	}
	//	psize = (int) phosphoColumns.size();
	//	ds << psize;
	//	for (int i = 0; i < psize; ++i) {
	//		ds << phosphoColumns[i];
	//	}
	//	psize = (int) madRanges.size();
	//	ds << psize;
	//	for (int i = 0; i < psize; ++i) {
	//		ds << madRanges[i];
	//	}
	ds << image;
}

void Processor::loadImage(const QString & imageFilePath, bool inverted) {
	image = QImage();
	if (image.load(imageFilePath)) {
		if (!isImageValid()) {
			return;
		}
		if (inverted) {
			image.invertPixels();
		}
		scanImage();
		emit imageChanged(&image);
	} else {
		emit message(tr("!! Failed loading image: \"%1\"").arg(imageFilePath));
	}
}

void Processor::scanImage() {
	int rows = image.height();
	int cols = image.width();
	RealMatrix & grayMatrix = matrices[MATRIX_GRAY_VALUE];
	RealMatrix & maskedMatrix = matrices[MATRIX_MASKED_GRAY_VALUE];
	grayMatrix.setDimension(rows, cols);
	maskedMatrix.setDimension(rows, cols, -1.0);
	grayMax = grayMin = qGray(image.pixel(0, 0));
	for (int r = 0; r < rows; ++r) {
		for (int c = 0; c < cols; ++c) {
			int color = qGray(image.pixel(c, r));
			grayMatrix.setValue(r, c, color);
			maskedMatrix.setValue(r, c, color);
			grayMax = grayMax > color ? grayMax : color;
			grayMin = grayMin < color ? grayMin : color;
		}
	}
	ready[MATRIX_GRAY_VALUE] = true;
	ready[MATRIX_MASKED_GRAY_VALUE] = true;
	emit matrixChanged(MATRIX_GRAY_VALUE, &grayMatrix);
	emit matrixChanged(MATRIX_MASKED_GRAY_VALUE, &maskedMatrix);
}

void Processor::setRowHeaders(const QStringList & rowHeaders) {
	setGridRowCount(rowHeaders.size());
	headers[HEADER_ROW] = rowHeaders;
//	syncOmittedHeaders(HEADER_ROW);
}

void Processor::setColHeaders(const QStringList & colHeaders) {
	setGridColumnCount(colHeaders.size());
	headers[HEADER_COL] = colHeaders;
//	syncOmittedHeaders(HEADER_COL);
}

const RealMatrix & Processor::getMatrix(MatrixType type) const {
	return matrices[type];
}

const QImage & Processor::getImage() const {
	return image;
}

void Processor::setMask(const QBitmap &mask) {
	int w = image.width();
	int h = image.height();
	QImage maskImage = mask.toImage();
	if (!(w == maskImage.width() && h == maskImage.height())) {
		return;
	}
	QRgb color1Rgb = QColor(Qt::color1).rgb();
	RealMatrix & maskedMatrix = matrices[MATRIX_MASKED_GRAY_VALUE];
	RealMatrix & grayMatrix = matrices[MATRIX_GRAY_VALUE];
	for (int r = 0; r < h; ++r) {
		for (int c = 0; c < w; ++c) {
			if (maskImage.pixel(c, r) != color1Rgb) {
				maskedMatrix.setValue(r, c, -1.0);
			} else {
				maskedMatrix.setValue(r, c, grayMatrix.getValue(r, c));
			}
		}
	}
	ready[MATRIX_MASKED_GRAY_VALUE] = true;
	emit Processor::matrixChanged(MATRIX_MASKED_GRAY_VALUE, &maskedMatrix);
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
	int curRows = headers[HEADER_ROW].size();
	if (rows == curRows) {
		return;
	}
	if (curRows > rows) {
		for (int r = 0; r < curRows - rows; ++r) {
			headers[HEADER_ROW].pop_back();
		}
	} else { // curRows < rows
		for (int r = 0; r < rows - curRows; ++r) {
			headers[HEADER_ROW].push_back(defaultRowHeaderName);
		}
	}
	calculateGrid();
	matrices[MATRIX_AVERAGES].setRowCount(rows);
	matrices[MATRIX_COUNTS].setRowCount(rows);
	ready[MATRIX_AVERAGES] = false;
	ready[MATRIX_COUNTS] = false;
//	for (int i = omittedRows.size() - 1; i >= 0; --i) {
//		if (omittedRows[i] >= rows) {
//			omittedRows.removeAt(i);
//		}
//	}
//	syncOmittedHeaders(HEADER_ROW);
	emit gridRowCountChanged(rows);
}

void Processor::setGridColumnCount(int cols) {
	int curCols = headers[HEADER_COL].size();
	if (cols == curCols) {
		return;
	}
	if (curCols > cols) {
		for (int c = 0; c < curCols - cols; ++c) {
			headers[HEADER_COL].pop_back();
		}
	} else { // curRows < rows
		for (int c = 0; c < cols - curCols; ++c) {
			headers[HEADER_COL].push_back(defaultColHeaderName);
		}
	}
	calculateGrid();
	matrices[MATRIX_AVERAGES].setColumnCount(cols);
	matrices[MATRIX_COUNTS].setColumnCount(cols);
	ready[MATRIX_AVERAGES] = false;
	ready[MATRIX_COUNTS] = false;
//	for (int i = omittedColumns.size() - 1; i >= 0; --i) {
//		if (omittedColumns[i] >= cols) {
//			omittedColumns.removeAt(i);
//		}
//	}
//	syncOmittedHeaders(HEADER_COL);
	emit gridColumnCountChanged(cols);
}

const Grid & Processor::getGrid() const {
	return grid;
}

const PatchList & Processor::getPatches() const {
	return patches;
}

int Processor::getGridRowCount() const {
	return headers[HEADER_ROW].size();
}

int Processor::getGridColumnCount() const {
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
	for (int r = 0; r < headers[HEADER_ROW].size(); ++r) {
		for (int c = 0; c < headers[HEADER_COL].size(); ++c) {
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

void Processor::setGridDimension(int rows, int cols) {
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
//	if (omittedRows.isEmpty()) {
		return headers[HEADER_ROW];
//	}
//	return omittedHeaders[HEADER_ROW];
}

const QStringList & Processor::getColumnHeaders() const {
//	if (omittedColumns.isEmpty()) {
		return headers[HEADER_COL];
//	}
//	return omittedHeaders[HEADER_COL];
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
			|| grid.getGeometries().size() == 0
			|| ready[MATRIX_MASKED_GRAY_VALUE] == false) {
		return;
	}
	RealMatrix & averages = matrices[MATRIX_AVERAGES];
	RealMatrix & counts = matrices[MATRIX_COUNTS];
	RealMatrix & masked = matrices[MATRIX_MASKED_GRAY_VALUE];
	double value;
	for (int i = 0; i < grid.size(); ++i) {
		double sum = .0;
		int count = 0;
		QRect rect = grid[i];
		for (int x = rect.left(); x < rect.right(); ++x) {
			if (x < 0 || x >= image.width()) {
				continue;
			}
			for (int y = rect.top(); y < rect.bottom(); ++y) {
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
		int r = i / headers[HEADER_COL].size();
		int c = i % headers[HEADER_COL].size();
		averages.setValue(r, c, sum / count);
		counts.setValue(r, c, count);
	}
	ready[MATRIX_AVERAGES] = true;
	ready[MATRIX_COUNTS] = true;
}

qint64 Processor::getGrayMax() const {
	return grayMax;
}

qint64 Processor::getGrayMin() const {
	return grayMin;
}

//void Processor::omitRow(int row) {
//	if (row >= 0 && row < headers[HEADER_ROW].size() && !omittedRows.contains(
//			row)) {
//		omittedRows.append(row);
//		qSort(omittedRows.begin(), omittedRows.end());
//		syncOmittedHeaders(HEADER_ROW);
//	}
//}

//void Processor::omitColumn(int column) {
//	if (column >= 0 && column < headers[HEADER_COL].size()
//			&& !omittedColumns.contains(column)) {
//		omittedColumns.append(column);
//		qSort(omittedColumns.begin(), omittedColumns.end());
//		syncOmittedHeaders(HEADER_COL);
//		ready[MATRIX_SCANSITE] = false;
//	}
//}

bool Processor::isReady(MatrixType type) const {
	return ready[type];
}

//void Processor::syncOmittedHeaders(HeaderType type) {
//	omittedHeaders[type] = headers[type];
//	QList<int> * omittedIndeces = 0;
//	switch (type) {
//	case HEADER_COL:
//		omittedIndeces = &omittedColumns;
//		break;
//	case HEADER_ROW:
//		omittedIndeces = &omittedRows;
//		break;
//	default:
//		emit message(tr("ERROR: no such header"));
//		return;
//	}
//	int index;
//	for (int i = omittedIndeces->size() - 1; i >= 0; --i) {
//		index = (*omittedIndeces)[i];
//		omittedHeaders[type].removeAt(index);
//	}
//}

QList<double> Processor::getAverageRow(int row) const {
	QList<double> returnList;
	if (ready[MATRIX_AVERAGES] == false || row < 0 || row
			>= headers[HEADER_ROW].size()) {
		return returnList;
	}
	for (int c = 0; c < matrices[MATRIX_AVERAGES].getColumnCount(); ++c) {
		//		if (omittedColumns.contains(c)) {
		//			continue;
		//		}
		returnList << matrices[MATRIX_AVERAGES].getValue(row, c);
	}
	return returnList;
}

QList<double> Processor::getAverageColumn(int column) const {
	QList<double> returnList;
	if (ready[MATRIX_AVERAGES] == false || column < 0 || column
			>= headers[HEADER_COL].size()) {
		return returnList;
	}
	for (int r = 0; r < matrices[MATRIX_AVERAGES].getRowCount(); ++r) {
//		if (omittedRows.contains(r)) {
//			continue;
//		}
		returnList << matrices[MATRIX_AVERAGES].getValue(r, column);
	}
	return returnList;
}

//void Processor::setMadRanges(const QList<double> & madRanges) {
//	this->madRanges = madRanges;
//}
//
//int Processor::getOriginalColumn(int omittedHeaderColumn) {
//	for (int i = 0; i < omittedColumns.size(); ++i) {
//		if (omittedHeaderColumn > omittedColumns[i]) {
//			++omittedHeaderColumn;
//		} else {
//			break;
//		}
//	}
//	return omittedHeaderColumn;
//}

const QStringList & Processor::getImageRowHeaders() const {
	return headers[HEADER_ROW];
}

const QStringList & Processor::getImageColumnHeaders() const {
	return headers[HEADER_COL];
}

double Processor::getGridAngle() const {
	return gridAngle;
}

//const QList<double> & Processor::getMadRanges() const {
//	return madRanges;
//}

//QStringList Processor::getPhosphoNames() const {
//	QStringList returnList;
//	for (int i = 0; i < phosphoColumns.size(); ++i) {
//		returnList << headers[HEADER_COL][phosphoColumns[i]];
//	}
//	return returnList;
//}

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

//void Processor::saveScansitePssm(const QString & filePath) {
//	if (!isReady(MATRIX_SCANSITE)) {
//		emit message(tr("!! Error: Scansite PSSM not calculated yet!"));
//		return;
//	}
//	QFile file(filePath);
//	if (file.open(QFile::WriteOnly) == false) {
//		emit message(tr("!! Cannot open file %1.").arg(filePath));
//		return;
//	}
//	QTextStream ds(&file);
//	QString hheader = getColumnHeaders().join("\t");
//	hheader += "\t*";
//	ds << hheader << "\n";
//	const RealMatrix & ssmatrix = matrices[MATRIX_SCANSITE];
//	int rows = ssmatrix.getRowCount();
//	for (int r = 0; r < rows; ++r) {
//		QString line;
//		int cols = ssmatrix.getColumnCount();
//		for (int c = 0; c < cols; ++c) {
//			line += QString::number(ssmatrix.getValue(r, c));
//			if (c < cols - 1) {
//				line += "\t";
//			}
//		}
//		ds << line << "\n";
//	}
//	emit message(tr("** Exported PSSM to \"%1\"").arg(filePath));
//}

void Processor::setMatrixValue(MatrixType type, int row, int column,
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
	if (!isReady(MATRIX_AVERAGES)) {
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
	const RealMatrix & mat = matrices[MATRIX_AVERAGES];
	int rows = mat.getRowCount();
	for (int r = 0; r < rows; ++r) {
		QString line = getImageRowHeaders()[r] + "\t";
		int cols = mat.getColumnCount();
		for (int c = 0; c < cols; ++c) {
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
		// they are all equal to 8-bit gray image
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

/*
 * MainWindow.cpp
 *
 *  Created on: Jun 2, 2010
 *      Author: Vincent
 */

#include "MainWindow.h"
#include "HeaderDialog.h"
#include <QtGui>
#include "DoubleDelegate.h"
#include <iostream>

#define MW_STATUSBAR_TRANSIENT 8000

MainWindow::MainWindow(QWidget * parent) :
	QMainWindow(parent) {
	ui.setupUi(this);
	transientPeriod = MW_STATUSBAR_TRANSIENT;
	ui.imageCanvas->installEventFilter(this);
	// qt connections
	makeConnections();
	// other initializations
	ui.imageOptionGroup->setEnabled(false);
	ui.overlapLabel->setVisible(false);
	updateProcessor();
	updatePatchType();
	ui.imageCanvas->setImageInverted(ui.actionShowInvertedImage->isChecked());
	ui.matrixTable->setItemDelegate(new DoubleDelegate(this));
	// restore settings
	loadSettings();
}

MainWindow::~MainWindow() {
}

void MainWindow::on_imageScaleSpin_valueChanged(double scale) {
	ui.imageCanvas->setScale(scale);
}

void MainWindow::updateTypeLabels() {
	QString orthoLabel, widthLabel;
	switch (ui.gridTypeCombo->currentIndex()) {
	case 0:
		orthoLabel = tr("Circle");
		ui.gridOrthoCheck->isChecked() ? widthLabel = tr("Radius") : widthLabel
				= tr("Width");
		processor.setGridType(Grid::GRID_ELLIPSE);
		break;
	case 1:
		orthoLabel = tr("Square");
		ui.gridOrthoCheck->isChecked() ? widthLabel = tr("Side") : widthLabel
				= tr("Width");
		processor.setGridType(Grid::GRID_RECTANGLE);
		break;
	default:
		processor.setGridType(Grid::GRID_NULL);
		break;
	}
	ui.gridOrthoCheck->setText(orthoLabel);
	ui.gridWidthLabel->setText(widthLabel);
	processor.setGridOrtho(ui.gridOrthoCheck->isChecked());
}

void MainWindow::updatePatchType() {
	if (ui.patchPolygonRadio->isChecked()) {
		patchType = Patch::PATCH_POLYGON;
	} else {
		patchType = Patch::PATCH_NULL;
	}
}

void MainWindow::on_rowColButton_clicked() {
	HeaderDialog hd(this);
	hd.setRowHeaders(processor.getImageRowHeaders());
	hd.setColumnHeaders(processor.getImageColumnHeaders());
	if (hd.exec() == QDialog::Accepted) {
		QStringList headers = hd.getHeaders();
		QStringList rowHeaders = headers[Processor::HEADER_ROW].split(QRegExp(
				"\\s+"), QString::SkipEmptyParts);
		QStringList colHeaders = headers[Processor::HEADER_COL].split(QRegExp(
				"\\s+"), QString::SkipEmptyParts);
		if (rowHeaders.size() > 0) {
			processor.setRowHeaders(rowHeaders);
		}
		if (colHeaders.size() > 0) {
			processor.setColHeaders(colHeaders);
		}
	}
	updateMatrix();
}

bool MainWindow::eventFilter(QObject * watched, QEvent * event) {
	// ellipse and rectangle patches not yet implemented
	if (watched == ui.imageCanvas) {
		if (event->type() == QEvent::MouseButtonPress) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent *> (event);
			// get the real coordinates on the image
			int x = trc(mouseEvent->x(), ui.imageScaleSpin->value());
			int y = trc(mouseEvent->y(), ui.imageScaleSpin->value());
			const QImage & im = processor.getImage();
			if (ui.patchGroup->isChecked()) {
				switch (patchType) {
				case Patch::PATCH_NULL:
				case Patch::PATCH_ELLIPSE:
				case Patch::PATCH_RECTANGLE:
					patchPolygon.clear();
					if (x < im.width() && y < im.height()) {
						showStatus(tr("(%1, %2) Gray %3").arg(x).arg(y).arg(
								qGray(im.pixel(x, y))));
					}
					break;
				case Patch::PATCH_POLYGON:
					patchPolygon << QPoint(x, y);
					showStatus(tr("(%1, %2) in polygon").arg(x).arg(y));
					break;
				}
			} else {
				if (x < im.width() && y < im.height()) {
					showStatus(tr("(%1, %2) Gray %3").arg(x).arg(y).arg(qGray(
							im.pixel(x, y))));
				}
			}
		} else if (event->type() == QEvent::MouseButtonDblClick) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent *> (event);
			int x = trc(mouseEvent->x(), ui.imageScaleSpin->value());
			int y = trc(mouseEvent->y(), ui.imageScaleSpin->value());
			// Because the MouseButtonPress event occurs before double click
			// here we don't need to add the point to the polygon again
			if (ui.patchGroup->isChecked()) {
				Patch patch(patchType);
				switch (patchType) {
				case Patch::PATCH_POLYGON:
					patch.setType(Patch::PATCH_POLYGON);
					patch.setPolygon(patchPolygon);
					processor.appendPatch(patch);
					patchPolygon = QPolygon();
					showStatus(tr("(%1, %2) ended polygon").arg(x).arg(y));
					break;
				default:
					break;
				}
			}
		}
	}
	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::initProject() {
	const QImage & image = processor.getImage();
	ui.gridHeightSpin->setRange(0, image.height() - 1);
	ui.gridHeightSpin->setValue(0);
	ui.gridWidthSpin->setRange(0, image.width() - 1);
	ui.gridWidthSpin->setValue(0);
	ui.gridStartXSpin->setRange(0, image.width() - 1);
	ui.gridStartXSpin->setValue(0);
	ui.gridStartYSpin->setRange(0, image.height() - 1);
	ui.gridStartYSpin->setValue(0);
	ui.gridEndXSpin->setRange(0, image.width() - 1);
	ui.gridEndXSpin->setValue(0);
	ui.gridEndYSpin->setRange(0, image.height() - 1);
	ui.gridEndYSpin->setValue(0);
	//	ui.gridGroup->setChecked(false);
	ui.imageCanvas->showGrid(ui.gridGroup->isChecked());
	//	ui.patchGroup->setChecked(false);
	ui.imageCanvas->showPatches(ui.patchGroup->isChecked());
	ui.imageOptionGroup->setEnabled(true);
	imageMasked = false;
}

void MainWindow::showStatus(const QString & message) {
	statusBar()->showMessage(message, transientPeriod);
	log(message);
}

int MainWindow::trc(int x, double factor) {
	return x / factor;
}

int MainWindow::tsc(int x, double factor) {
	return x * factor;
}

void MainWindow::on_actionLoadImage_triggered() {
	QString filePath = QFileDialog::getOpenFileName(this, tr("Load Image"),
			lastImagePath, tr("Images (*.png *.jpg *.jpeg *.bmp "
				"*.tif *.tiff *.gif *.xbm *.xpm)"));
	if (filePath.isEmpty()) {
		return;
	}
	lastImagePath = filePath;
	processor.loadImage(filePath);
	initProject();
	log(tr("Size:   %1x%2").arg(processor.getImage().width()).arg(
			processor.getImage().height()));
	log(tr("Format: %1").arg(getFormatName(processor.getImage().format())));
	showStatus(tr("** Loaded image \"%1\"").arg(filePath));
}

void MainWindow::updateProcessorStartPoint() {
	processor.setGridStartPoint(QPoint(ui.gridStartXSpin->value(),
			ui.gridStartYSpin->value()));
}

void MainWindow::updateProcessorEndPoint() {
	processor.setGridEndPoint(QPoint(ui.gridEndXSpin->value(),
			ui.gridEndYSpin->value()));
}

void MainWindow::updateProcessor() {
	processor.setGridAngle(ui.gridAngleSpin->value());
	processor.setGridDimension(ui.gridRowSpin->value(),
			ui.gridColumnSpin->value());
	processor.setGridElementHeight(ui.gridHeightSpin->value());
	processor.setGridElementWidth(ui.gridWidthSpin->value());
	updateTypeLabels();
	updateProcessorStartPoint();
	updateProcessorEndPoint();
}

void MainWindow::closeEvent(QCloseEvent * event) {
	saveSettings();
	event->accept();
}

void MainWindow::on_canvasSensitivitySlider_valueChanged(int sensitivityValue) {
	ui.inspectorCanvas->setSensitivity(sensitivityValue / 100.0);
}

void MainWindow::on_canvasScaleSlider_valueChanged(int scaleValue) {
	ui.inspectorCanvas->setUnitScale(scaleValue / 100.0);
}

void MainWindow::on_actionApplyMask_triggered() {
	imageMasked = true;
	processor.setMask(ui.imageCanvas->getMask());
	updateInspector();
}

void MainWindow::updateInspector() {
	if (processor.getImage().isNull()) {
		return;
	}
	ui.inspectorCanvas->setMaxColor(processor.getGrayMax());
	ui.inspectorCanvas->setMinColor(processor.getGrayMin());
	ui.inspectorCanvas->setUpperBorder(processor.getUpperBorder());
	ui.inspectorCanvas->setLowerBorder(processor.getLowerBorder());
	if (imageMasked) {
		if (!processor.isReady(Processor::MATRIX_MASKED_GRAY_VALUE)) {
			return;
		}
		ui.inspectorCanvas->setMatrix(&processor.getMatrix(
				Processor::MATRIX_MASKED_GRAY_VALUE));
	} else {
		if (!processor.isReady(Processor::MATRIX_GRAY_VALUE)) {
			return;
		}
		ui.inspectorCanvas->setMatrix(&processor.getMatrix(
				Processor::MATRIX_GRAY_VALUE));
	}
}

void MainWindow::on_actionInvertImage_triggered() {
	processor.invertImage();
}

void MainWindow::log(const QString &message) {
	ui.consoleBrowser->append(message);
}

QString MainWindow::getFormatName(QImage::Format format) {
	switch (format) {
	case QImage::Format_Mono:
		return tr("1-bit/pixel image");
		break;
	case QImage::Format_Indexed8:
		return tr("8-bit indexed colormap.");
		break;
	case QImage::Format_RGB16:
		return tr("16-bit RGB format (5-6-5)");
		break;
	case QImage::Format_RGB32:
		return tr("32-bit RGB format (0xffRRGGBB)");
		break;
	case QImage::Format_ARGB32:
		return tr("32-bit ARGB format (0xAARRGGBB)");
		break;
	case QImage::Format_Invalid:
		return tr("Invalid");
		break;
	default:
		return tr("Unknown");
		break;
	}
}

void MainWindow::on_actionToggleMask_triggered() {
	imageMasked = !imageMasked;
	processor.setMask(ui.imageCanvas->getMask());
	updateInspector();
}

QStringList MainWindow::getResidues(QPlainTextEdit * edit) const {
	QStringList toBeProcessed = edit->toPlainText().split(QRegExp("\\s+"),
			QString::SkipEmptyParts);
	QStringList returnList;
	for (int i = 0; i < toBeProcessed.size(); ++i) {
		if (processor.getColumnHeaders().contains(toBeProcessed[i])) {
			returnList << toBeProcessed[i];
		}
	}
	return returnList;
}

void MainWindow::updateMatrix() {
	if (!processor.isReady(Processor::MATRIX_AVERAGES)) {
		processor.calculateAverages();
	}
	displayMatrix(processor.getMatrix(Processor::MATRIX_AVERAGES));
	ui.matrixTable->setVerticalHeaderLabels(processor.getImageRowHeaders());
	ui.matrixTable->setHorizontalHeaderLabels(processor.getImageColumnHeaders());
}

void MainWindow::displayMatrix(const RealMatrix & matrix) {
	ui.matrixTable->setColumnCount(matrix.getColumnCount());
	ui.matrixTable->setRowCount(matrix.getRowCount());
	QTableWidgetItem * item;
	for (int r = matrix.getRowCount() - 1; r >= 0; --r) {
		for (int c = matrix.getColumnCount() - 1; c >= 0; --c) {
			item = new QTableWidgetItem;
			item->setData(Qt::DisplayRole, matrix.getValue(r, c));
			ui.matrixTable->setItem(r, c, item);
		}
	}
}

void MainWindow::on_actionLoadProject_triggered() {
	QString filePath = QFileDialog::getOpenFileName(this, tr("Load project"),
			lastProjectPath, tr("ARIAS project files (*.arp)"));
	if (filePath.isEmpty()) {
		return;
	}
	lastProjectPath = filePath;
	if (!processor.loadProject(filePath)) {
		return;
	}
	showStatus(tr("** Loaded project \"%1\"").arg(filePath));
	initProject();
	ui.gridRowSpin->setValue(processor.getImageRowHeaders().size());
	ui.gridColumnSpin->setValue(processor.getImageColumnHeaders().size());
	ui.imageCanvas->setGrid(&processor.getGrid());
	ui.imageCanvas->setPatches(&processor.getPatches());
	ui.gridAngleSpin->setValue(processor.getGridAngle());
	ui.gridWidthSpin->setValue(processor.getGridElementWidth());
	ui.gridHeightSpin->setValue(processor.getGridElementHeight());
	ui.gridStartXSpin->setValue(processor.getGridStartPoint().x());
	ui.gridStartYSpin->setValue(processor.getGridStartPoint().y());
	ui.gridEndXSpin->setValue(processor.getGridEndPoint().x());
	ui.gridEndYSpin->setValue(processor.getGridEndPoint().y());
	ui.imageCanvas->setImage(&processor.getImage());
	ui.imageCanvas->setGrid(&processor.getGrid());
	ui.imageCanvas->setPatches(&processor.getPatches());
	ui.actionCalculateAverages->trigger();
	updateInspector();
	updateMatrix();
}

void MainWindow::on_actionSaveProject_triggered() {
	QString filePath = QFileDialog::getSaveFileName(this, tr("Load project"),
			lastProjectPath, tr("ARIAS project files (*.arp)"));
	if (filePath.isEmpty()) {
		return;
	}
	if (!filePath.endsWith(".arp")) {
		filePath += ".arp";
	}
	lastProjectPath = filePath;
	processor.saveProject(filePath);
	showStatus(tr("** Project saved to \"%1\"").arg(filePath));
}

void MainWindow::loadSettings() {
	QSettings settings;
	settings.beginGroup("GUI");
	if (settings.contains("imageTabSplitterState")) {
		ui.imageTabSplitter->restoreState(settings.value(
				"imageTabSplitterState").toByteArray());
	}
	if (settings.contains("orthoGrid")) {
		ui.gridOrthoCheck->setChecked(settings.value("orthoGrid").toBool());
	}
	if (settings.contains("inspectorTabSplitterState")) {
		ui.inspectorTabSplitter->restoreState(settings.value(
				"inspectorTabSplitterState").toByteArray());
	}
	if (settings.contains("windowGeometry")) {
		restoreGeometry(settings.value("windowGeometry").toByteArray());
	}
	if (settings.contains("windowState")) {
		restoreState(settings.value("windowState").toByteArray());
	}
	ui.tabWidget->setCurrentIndex(settings.value("currentTab", 0).toInt());
	if (settings.contains("imageShownInverted")) {
		ui.actionShowInvertedImage->setChecked(settings.value(
				"imageShownInverted", true).toBool());
	}
	settings.endGroup();
	settings.beginGroup("Variables");
	lastImagePath
			= settings.value("lastImagePath", QDir::homePath()).toString();
	lastProjectPath
			= settings.value("lastProjectPath", QDir::homePath()).toString();
	lastExportPath
			= settings.value("lastExportPath", QDir::homePath()).toString();
	if (settings.contains("rowHeaders")) {
		processor.setRowHeaders(settings.value("rowHeaders").toStringList());
	}
	if (settings.contains("columnHeaders")) {
		processor.setColHeaders(settings.value("columnHeaders").toStringList());
	}
	settings.endGroup();
}

void MainWindow::saveSettings() {
	QSettings settings;
	settings.beginGroup("GUI");
	settings.setValue("imageTabSplitterState", ui.imageTabSplitter->saveState());
	settings.setValue("orthoGrid", ui.gridOrthoCheck->isChecked());
	settings.setValue("inspectorTabSplitterState",
			ui.inspectorTabSplitter->saveState());
	settings.setValue("currentTab", ui.tabWidget->currentIndex());
	settings.setValue("windowGeometry", saveGeometry());
	settings.setValue("windowState", saveState());
	settings.setValue("imageShownInverted",
			ui.actionShowInvertedImage->isChecked());
	settings.endGroup();
	settings.beginGroup("Variables");
	settings.setValue("lastImagePath", lastImagePath);
	settings.setValue("lastProjectPath", lastProjectPath);
	settings.setValue("lastExportPath", lastExportPath);
	settings.setValue("rowHeaders", processor.getImageRowHeaders());
	settings.setValue("columnHeaders", processor.getImageColumnHeaders());
	settings.endGroup();
	settings.sync();
}

void MainWindow::debugMatrix(const RealMatrix * matrix) {
	for (int r = matrix->getRowCount() - 1; r >= 0; --r) {
		for (int c = matrix->getColumnCount() - 1; c >= 0; --c) {
			std::cout << matrix->getValue(r, c) << " ";
		}
		std::cout << "\n";
	}
}

void MainWindow::debug(const QString &message) {
	std::cout << "DEBUG: " << message.toStdString() << std::endl;
}

void MainWindow::makeConnections() {
	connect(ui.patchGroup, SIGNAL(toggled(bool)), ui.imageCanvas, SLOT(showPatches(bool)));
	connect(ui.patchNullRadio, SIGNAL(toggled(bool)), this, SLOT(updatePatchType()));
	connect(ui.patchPolygonRadio, SIGNAL(toggled(bool)), this, SLOT(updatePatchType()));
	connect(ui.imageScaleSpin, SIGNAL(valueChanged(double)), ui.imageCanvas, SLOT(setScale(double)));
	connect(ui.gridGroup, SIGNAL(toggled(bool)), ui.imageCanvas, SLOT(showGrid(bool)));
	connect(ui.gridTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTypeLabels()));
	connect(ui.gridOrthoCheck, SIGNAL(toggled(bool)), this, SLOT(updateTypeLabels()));
	connect(ui.gridStartXSpin, SIGNAL(valueChanged(double)), &processor, SLOT(setGridStartPointX(double)));
	connect(ui.gridStartYSpin, SIGNAL(valueChanged(double)), &processor, SLOT(setGridStartPointY(double)));
	connect(ui.gridEndXSpin, SIGNAL(valueChanged(double)), &processor, SLOT(setGridEndPointX(double)));
	connect(ui.gridEndYSpin, SIGNAL(valueChanged(double)), &processor, SLOT(setGridEndPointY(double)));
	connect(ui.actionShowInvertedImage, SIGNAL(toggled(bool)), ui.imageCanvas, SLOT(setImageInverted(bool)));
	connect(ui.actionLookDownZ, SIGNAL(toggled(bool)), ui.inspectorCanvas, SLOT(setLookDownZ(bool)));
	connect(ui.actionOrthoView, SIGNAL(toggled(bool)), ui.inspectorCanvas, SLOT(setOrthoView(bool)));
	//	connect(ui.actionResetView, SIGNAL(triggered()), ui.inspectorCanvas, SLOT(
	//			resetView()));
	connect(ui.gridAngleSpin, SIGNAL(valueChanged(double)), &processor, SLOT(setGridAngle(double)));
	connect(ui.gridRowSpin, SIGNAL(valueChanged(int)), &processor, SLOT(setGridRowCount(int)));
	connect(ui.gridColumnSpin, SIGNAL(valueChanged(int)), &processor, SLOT(setGridColumnCount(int)));
	connect(ui.gridWidthSpin, SIGNAL(valueChanged(double)), &processor, SLOT(setGridElementWidth(double)));
	connect(ui.gridHeightSpin, SIGNAL(valueChanged(double)), &processor, SLOT(setGridElementHeight(double)));
	connect(ui.actionClearPatches, SIGNAL(triggered()), &processor, SLOT(
			clearPatches()));
	connect(ui.actionClearLastPatch, SIGNAL(triggered()), &processor, SLOT(
			clearLastPatch()));
	connect(&processor, SIGNAL(message(const QString &)), this, SLOT(showStatus(const QString &)));
	connect(&processor, SIGNAL(matrixChanged(int, RealMatrix *)), this, SLOT(updateInspector()));
	connect(&processor, SIGNAL(imageChanged(const QImage *)), ui.imageCanvas, SLOT(setImage(const QImage *)));
	connect(&processor, SIGNAL(patchListChanged(const PatchList *)), ui.imageCanvas, SLOT(setPatches(const PatchList *)));
	connect(&processor, SIGNAL(gridChanged(const Grid *)), ui.imageCanvas, SLOT(setGrid(const Grid *)));
	connect(&processor, SIGNAL(gridRowCountChanged(int)), ui.gridRowSpin, SLOT(setValue(int)));
	connect(&processor, SIGNAL(gridColumnCountChanged(int)), ui.gridColumnSpin, SLOT(setValue(int)));
	connect(&processor, SIGNAL(gridAngleChanged(double)), ui.gridAngleSpin, SLOT(setValue(double)));
	connect(&processor, SIGNAL(gridElementWidthChanged(double)), ui.gridWidthSpin, SLOT(setValue(double)));
	connect(&processor, SIGNAL(gridElementHeightChanged(double)), ui.gridHeightSpin, SLOT(setValue(double)));
	connect(&processor, SIGNAL(gridStartXChanged(double)), ui.gridStartXSpin, SLOT(setValue(double)));
	connect(&processor, SIGNAL(gridStartYChanged(double)), ui.gridStartYSpin, SLOT(setValue(double)));
	connect(&processor, SIGNAL(gridEndXChanged(double)), ui.gridEndXSpin, SLOT(setValue(double)));
	connect(&processor, SIGNAL(gridEndYChanged(double)), ui.gridEndYSpin, SLOT(setValue(double)));
	connect(&processor, SIGNAL(gridOverlapped(bool)), ui.overlapLabel, SLOT(setVisible(bool)));
	connect(ui.actionCalculateAverages, SIGNAL(triggered()), this, SLOT(
			calculateAverages()));
}

void MainWindow::on_actionExportAverageIntensities_triggered() {
	QString filePath = QFileDialog::getSaveFileName(this, tr(
			"Export average intensities"), lastExportPath, tr(
			"Tab-delimited text files (*.txt)"));
	if (filePath.isEmpty()) {
		return;
	}
	lastExportPath = filePath;
	processor.saveAverageIntensities(filePath);
}

void MainWindow::on_actionResetView_triggered() {
	ui.actionLookDownZ->setChecked(false);
	ui.actionOrthoView->setChecked(false);
	ui.inspectorCanvas->resetView();
}

void MainWindow::calculateAverages() {
	processor.calculateAverages();
	updateMatrix();
}

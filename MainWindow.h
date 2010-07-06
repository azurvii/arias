/*
 * MainWindow.h
 *
 *  Created on: Jun 2, 2010
 *      Author: Vincent
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include "ui_MainWindow.h"
#include "Processor.h"
#include "Patch.h"

class QPlainTextEdit;

class MainWindow: public QMainWindow {
Q_OBJECT
public:
	MainWindow(QWidget * parent = 0);
	virtual ~MainWindow();

public slots:
	void showStatus(const QString & message);
	void updatePatchType();
	void on_imageScaleSpin_valueChanged(double scale);
	void on_rowColButton_clicked();
	void on_actionLoadImage_triggered();
	void on_actionLoadProject_triggered();
	void on_actionSaveProject_triggered();
	void on_actionExportCurrentMatrix_triggered();
	void on_canvasSensitivitySlider_valueChanged(int sensitivityValue);
	void on_canvasScaleSlider_valueChanged(int scaleValue);
	void on_actionInvertImage_triggered();
	void on_actionApplyMask_triggered();
	void on_actionToggleMask_triggered();
	void on_actionResetView_triggered();
	void on_actionWhiteBackground_toggled(bool white);
	void updateTypeLabels();
	void updateProcessorStartPoint();
	void updateProcessorEndPoint();
	void updateInspector();
	void updateProcessor();
	void updateMatrix();
	void displayMatrix(const RealMatrix & matrix);
	void log(const QString & message);
	void calculateAverages();
	void updateChannel();

protected:
	void closeEvent(QCloseEvent * event);
	void initProject();
	bool eventFilter(QObject * watched, QEvent * event);
	// Transform to real coordinate
	inline qint32 trc(qint32 x, double factor);
	// Transform to screen coordinate
	inline qint32 tsc(qint32 x, double factor);

private:
	static QString getFormatName(QImage::Format format);
	QStringList getResidues(QPlainTextEdit * edit) const;
	void loadSettings();
	void saveSettings();
	void makeConnections();
	void debugMatrix(const RealMatrix * matrix);
	void debug(const QString &message);

private:
	Ui::MainWindow ui;
	Processor processor;
	qint32 transientPeriod;
	Patch::PatchType patchType;
	QPolygon patchPolygon;
	QString lastImagePath;
	QString lastProjectPath;
	QString lastExportPath;
	bool imageMasked;
};

#endif /* MAINWINDOW_H_ */

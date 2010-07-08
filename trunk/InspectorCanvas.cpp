/*
 * InspectorCanvas.cpp
 *
 *  Created on: Jun 2, 2010
 *      Author: Vincent
 */

#include "InspectorCanvas.h"
#include <QtGui>
#include <QtOpenGL>

#define DEFAULT_EYE_X -10.0
#define DEFAULT_EYE_Y -10.0
#define DEFAULT_EYE_Z 40.0
#define DEFAULT_CENTER_X 100.0
#define DEFAULT_CENTER_Y 100.0
#define DEFAULT_CENTER_Z .0
#define DEFAULT_SENSITIVIY .2
#define DEFAULT_UNIT_SCALE .1

InspectorCanvas::InspectorCanvas(QWidget *parent) :
	QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::DoubleBuffer | QGL::Rgba
			| QGL::DepthBuffer), parent) {
	eyeX = DEFAULT_EYE_X;
	eyeY = DEFAULT_EYE_Y;
	eyeZ = DEFAULT_EYE_Z;
	focusX = DEFAULT_CENTER_X;
	focusY = DEFAULT_CENTER_Y;
	focusZ = DEFAULT_CENTER_Z;
	sensitivity = DEFAULT_SENSITIVIY;
	imageScale = DEFAULT_UNIT_SCALE;
	zScale = 1.0;
	imageShown = true;
	lookZ = false;
	ortho = false;
	colorMax = -1;
	colorMin = -1;
	borderLower = -1;
	borderUpper = -1;
	renderingMatrix = 0;
	guideLimit = 500.0;
	axisLimit = 10000.0;
	tickInterval = 20.0;
	// axisXColor is red
	axisXColor[0] = .3f;
	axisXColor[1] = .0f;
	axisXColor[2] = .0f;
	// axisYColor is green
	axisYColor[0] = .0f;
	axisYColor[1] = .3f;
	axisYColor[2] = .0f;
	// axisZColor is blue
	axisZColor[0] = .0f;
	axisZColor[1] = .0f;
	axisZColor[2] = .3f;
}

InspectorCanvas::~InspectorCanvas() {
}

void InspectorCanvas::setSensitivity(double sensitivity) {
	if (this->sensitivity != sensitivity) {
		this->sensitivity = sensitivity;
		emit sensitivityChanged(sensitivity);
		update();
	}
}

void InspectorCanvas::setUnitScale(double scale) {
	if (imageScale != scale) {
		imageScale = scale;
		emit unitScaleChanged(scale);
		update();
	}
}

void InspectorCanvas::setLookDownZ(bool lookZ) {
	this->lookZ = lookZ;
	update();
}

void InspectorCanvas::setOrthoView(bool ortho) {
	this->ortho = ortho;
	resizeGL(width(), height());
	update();
}

void InspectorCanvas::initializeGL() {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//    glClearDepth(1.0f);
	glEnable( GL_DEPTH_TEST);
	glShadeModel( GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
#ifdef Q_WS_MAC
	glEnable(GL_MULTISAMPLE);
#endif
}

void InspectorCanvas::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	if (lookZ) {
		gluLookAt(eyeX, eyeY, eyeZ, eyeX, eyeY, eyeZ - 10.0, .0, 1.0, .0);
	} else {
		gluLookAt(eyeX, eyeY, eyeZ, focusX, focusY, focusZ, .0, .0, 1.0);
	}
	// draw the axis
	glBegin( GL_LINES);
	// x
	glColor3fv(axisXColor);
	glVertex3f(-axisLimit, .0f, .0f);
	//  glColor3f(.5f, .5f, .0f);
	glVertex3f(axisLimit, .0f, .0f);
	glColor3f(.2f, .2f, .2f);
	for (qint32 i = tickInterval; i < guideLimit; i += tickInterval) {
		glVertex3f(-guideLimit, i, .0f);
		glVertex3f(guideLimit, i, .0f);
		glVertex3f(-guideLimit, -i, .0f);
		glVertex3f(guideLimit, -i, .0f);
	}
	// y
	glColor3f(.0f, .3f, .0f);
	glVertex3f(.0f, -axisLimit, .0f);
	glVertex3f(.0f, axisLimit, .0f);
	glColor3f(.2f, .2f, .2f);
	for (qint32 i = tickInterval; i < guideLimit; i += tickInterval) {
		glVertex3f(i, -guideLimit, .0f);
		glVertex3f(i, guideLimit, .0f);
		glVertex3f(-i, -guideLimit, .0f);
		glVertex3f(-i, guideLimit, .0f);
	}
	// z
	glColor3f(.0f, .0f, .3f);
	glVertex3f(.0f, .0f, -axisLimit);
	glVertex3f(.0f, .0f, axisLimit);
	glEnd(/* GL_LINES */);
	if (imageShown && renderingMatrix) {
		renderImage();
	}
	swapBuffers();
}

void InspectorCanvas::resizeGL(qint32 width, qint32 height) {
	glViewport(0, 0, width, height);

	glMatrixMode( GL_PROJECTION); // Select The Projection Matrix
	glLoadIdentity(); // Reset The Projection Matrix

	if (ortho) {
		glOrtho(-width * imageScale, width * imageScale, -height * imageScale,
				height * imageScale, -10000.0, 10000.0);
	} else {
		// Calculate The Aspect Ratio Of The Window
		gluPerspective(45.0, (GLdouble) width / (GLdouble) height, 0.1, 10000.0);
	}

	glMatrixMode( GL_MODELVIEW); // Select The Modelview Matrix
	glLoadIdentity(); // Reset The Modelview Matrix
}

void InspectorCanvas::setEye(double x, double y, double z) {
	eyeX = x;
	eyeY = y;
	eyeZ = z;
}

void InspectorCanvas::setFocus(double x, double y, double z) {
	focusX = x;
	focusY = y;
	focusZ = z;
}

void InspectorCanvas::resetView() {
	setEye(DEFAULT_EYE_X, DEFAULT_EYE_Y, DEFAULT_EYE_Z);
	setFocus(DEFAULT_CENTER_X, DEFAULT_CENTER_Y, DEFAULT_CENTER_Z);
	setSensitivity(DEFAULT_SENSITIVIY);
	setUnitScale(DEFAULT_UNIT_SCALE);
	update();
}

void InspectorCanvas::mouseMoveEvent(QMouseEvent *event) {
	qint32 dx = event->x() - lastPos.x();
	qint32 dy = event->y() - lastPos.y();

	if (event->buttons() & Qt::LeftButton) {
		double localSense = sensitivity * .01;
		setFocus((focusX - eyeX) * cos(-dx * localSense) - (focusY - eyeY)
				* sin(-dx * localSense) + eyeX, (focusX - eyeX) * sin(-dx
				* localSense) + (focusY - eyeY) * cos(-dx * localSense) + eyeY,
				focusZ - dy * sensitivity);
	} else if (event->buttons() & Qt::RightButton) {
		double localSense = sensitivity * .5;
		double moveX;
		double moveY;
		if (lookZ) {
			moveX = -dx * localSense;
			moveY = dy * localSense;
		} else {
			double X = focusX - eyeX;
			double Y = focusY - eyeY;
			double longEdge = sqrt(X * X + Y * Y);
			moveX = (dy * X - dx * Y) / longEdge * localSense;
			moveY = (dy * Y + dx * X) / longEdge * localSense;
		}
		setEye(eyeX + moveX, eyeY + moveY, eyeZ);
		setFocus(focusX + moveX, focusY + moveY, focusZ);
		//      }
	}
	lastPos = event->pos();
	update();
}

void InspectorCanvas::mousePressEvent(QMouseEvent *event) {
	lastPos = event->pos();
}

void InspectorCanvas::wheelEvent(QWheelEvent * event) {
	double dz = event->delta();
	setEye(eyeX, eyeY, eyeZ - dz * sensitivity / 5.0);
	update();
}

void InspectorCanvas::renderImage() {
	if (imageShown) {
		qint32 colorRange = colorMax - colorMin;
		glBegin( GL_POINTS);
		qint32 rows = renderingMatrix->getRowCount();
		qint32 cols = renderingMatrix->getColumnCount();
		for (qint32 r = 0; r < rows; ++r) {
			for (qint32 c = 0; c < cols; ++c) {
				double value = renderingMatrix->getValue(r, c);
				if (value >= 0) {
					if (borderLower == -1 || borderUpper == -1 || colorMax
							== -1 || colorMin == -1) {
						glColor3f(.0f, .7f, .0f);
					} else if (value == borderLower || value == borderUpper) {
						glColor3f(.7f, .0f, .0f);
					} else {
						glColor3f((GLfloat) (value - colorMin) / colorRange,
								.7f, (GLfloat) (colorMax - value) / colorRange);
					}
					glVertex3f(c * imageScale, (rows - r) * imageScale, value
							* imageScale * zScale);
				}
			}
		}
		glEnd();
	}
}

double InspectorCanvas::getSensitivity() const {
	return sensitivity;
}

double InspectorCanvas::getImageScale() const {
	return imageScale;
}

double InspectorCanvas::getZScale() const {
	return zScale;
}

bool InspectorCanvas::isImageShown() const {
	return imageShown;
}

void InspectorCanvas::setMatrix(const RealMatrix * matrix) {
	renderingMatrix = matrix;
	update();
}

void InspectorCanvas::setMaxColor(qint32 color) {
	colorMax = color;
}

void InspectorCanvas::setMinColor(qint32 color) {
	colorMin = color;
}

void InspectorCanvas::setUpperBorder(qint32 color) {
	borderUpper = color;
}

void InspectorCanvas::setLowerBorder(qint32 color) {
	borderLower = color;
}

void InspectorCanvas::setZScale(double scale) {
	zScale = scale;
}

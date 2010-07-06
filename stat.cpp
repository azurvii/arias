/*
 * stat.cpp
 *
 *  Created on: Apr 28, 2010
 *      Author: Vincent
 */

#include "stat.h"
#include <cmath>

int minIndexOf(const QList<double> &list) {
	int minIndex = 0;
	for (int i = 0; i < list.size(); ++i) {
		if (list[minIndex] > list[i]) {
			minIndex = i;
		}
	}
	return minIndex;
}

double median(QList<double> data) {
	qSort(data);
	int n = data.size() / 2;
	if (data.size() % 2 != 0 && n != 0) {
		return (data[n] + data[n + 1]) / 2.0;
	} else {
		return data[n];
	}
}

double mean(const QList<double> &data) {
	double sum = 0.0;
	for (int i = 0; i < data.size(); ++i) {
		sum += data[i];
	}
	return sum / data.size();
}

double mad(const QList<double> &data) {
	double med = median(data);
	QList<double> absDiff;
	for (int i = 0; i < data.size(); ++i) {
		absDiff << std::abs(data[i] - med);
	}
	return median(absDiff);
}


/*
 * stat.h
 *
 *  Created on: Apr 28, 2010
 *      Author: Vincent
 *
 * Some shared functions
 *
 */

#ifndef STAT_H_
#define STAT_H_

#include <QList>

extern qint32 minIndexOf(const QList<double> &list);
extern double median(QList<double> data);
extern double mean(const QList<double> &data);
extern double mad(const QList<double> &data);

#endif /* STAT_H_ */

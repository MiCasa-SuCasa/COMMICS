/****************************************************************************
** File: compclusteringengine.h
** Author( s ): Roberto Garcia Yunta( robergy at gmail.com )
** Creation Date: 2010-10-31
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
****************************************************************************/
#ifndef COMPCLUSTERINGENGINE_H
#define COMPCLUSTERINGENGINE_H

#include <QMainWindow>
#include <QMap>
#include <QPair>

#include <iostream>



class CompClusteringEngine
{

public:

    CompClusteringEngine( std::string );

    int getDataImgHeight();

    int getDataImgWidth();

    std::string getErrorString();

    std::string paintClusteringData();

    std::string paintCogs( int theYcoordinate, int theHeight );

    std::string paintHorizontalTree();

    std::string paintMetagenomes();

    std::string paintScale();

    std::string paintVerticalTree();

    std::string paintZoomImage( int theYcoordinate, int theHeight );

    bool readFile( std::string theFileName );

private:

    void calculateSizes();

    QString maxString( QVector<QString> );

    void setMaxMinValues();

    QVector<QString> cvCogNames;

    QVector<QVector<double> > cvDataMatrix;

    double cvMaxPosValue, cvMinPosValue, cvMaxNegValue, cvMinNegValue;

    QVector<QString> cvMetagenomeNames;

    int cvRectangleWidth;

    int cvRectangleHeight;

    QMap<QString,QPair<double,double> > cvHTreeNodes;

    QMap<QString,QPair<double,double> > cvVTreeNodes;

    QMap<QString,QPair<QString,QString> >cvChildNodes;

    QString cvTimeStamp;

    QString cvFileName; //file name without extension

    QString cvFilePath;

    QString cvErrorString;

    QString cvPath2Images;

    int cvDataImgWidth;

    int cvDataImgHeight;
};

#endif


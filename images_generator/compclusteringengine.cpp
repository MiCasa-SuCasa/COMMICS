/****************************************************************************
** File: compclusteringengine.cpp
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

#include <limits>
#include <cmath>

#include <QDateTime>
#include <QPainter>
#include <QDir>
#include <QTextStream>
#include <QApplication>

#include "compclusteringengine.h"
#define ZOOMED_SQUARE_SIZE 14

CompClusteringEngine::CompClusteringEngine( std::string thePath )
{
    cvTimeStamp = QDateTime( QDateTime::currentDateTime() ).toString( "hhmmsszzz" );
    cvErrorString = "Everything is cool";
    cvPath2Images = QString( thePath.c_str() );
    cvDataImgWidth = 0;
    cvDataImgHeight = 0;
    cvRectangleWidth = 3;
    cvRectangleHeight = 3;
}

int CompClusteringEngine::getDataImgHeight()
{
  return cvDataImgHeight;
}

int CompClusteringEngine::getDataImgWidth()
{
  return cvDataImgWidth;
}

std::string CompClusteringEngine::getErrorString()
{
    return cvErrorString.toStdString();
}

std::string CompClusteringEngine::paintClusteringData()
{

    QImage baseImage( cvDataImgWidth, cvDataImgHeight,QImage::Format_RGB32 );

    baseImage.fill( 0xFFFFFF );
    QPainter painter( &baseImage );
    QPen highlightedPen( QColor( 255,255,255 ) );
    painter.setPen(highlightedPen);

    for( int i = 0; i < this->cvDataMatrix.size(); i++ )
        for( int j = 0; j < this->cvDataMatrix[0].size(); j++ )
        {
	    if(  std::isnan(cvDataMatrix[i][j])  ) //there may be unset values in the cdt file
	    {
	    	QColor color = QColor(170,170,170, 255);
                painter.setBrush(QBrush(color));
                //
                painter.setPen(QColor(170,170,170,0));
	    }
	    else if(cvMinPosValue <= cvDataMatrix[i][j] && cvDataMatrix[i][j] <= cvMaxPosValue)
            {
               //int lvAlpha = 255*((cvDataMatrix[i][j] - cvMinPosValue )/(cvMaxPosValue-cvMinPosValue) );
               int lvAlpha = 255* (cvDataMatrix[i][j]/cvMaxPosValue);
               QColor color = QColor(255,0,0,lvAlpha);
               painter.setBrush(QBrush(color));
               //
               painter.setPen(QColor(255,0,0,0));
            }
            else
            {
               //int lvAlpha = 255*((-cvDataMatrix[i][j] +cvMinPosValue )/(-cvMaxPosValue+cvMinPosValue) );
               int lvAlpha = 255*(cvDataMatrix[i][j]/cvMinNegValue);
               QColor color = QColor(0,0,255, lvAlpha);
               painter.setBrush(QBrush(color));
               //
               painter.setPen(QColor(0,0,255,0));
            }
            painter.drawRect((j*cvRectangleWidth),i*cvRectangleHeight,cvRectangleWidth,cvRectangleHeight);

        }//for

    // save the result image
    QString lvFileName = cvFileName+"_cluster_data_"+cvTimeStamp+".png";
    QDate lvCurrentDate = QDate::currentDate();
    QString lvImgsDir = "images"+lvCurrentDate.toString( "yyyyMMdd" );
    QDir lvCurrentDir( cvPath2Images );
    if( ! lvCurrentDir.exists( lvImgsDir ) )
    {
        lvCurrentDir.mkdir( lvImgsDir );
    }
    QString lvPath2File = lvImgsDir+"/"+lvFileName;
    bool errorCode = baseImage.save( lvCurrentDir.path()+"/"+lvPath2File, "PNG" );
    if(! errorCode)
    {
        cvErrorString = "Error saving cluster data image";
        painter.end();
        return "";
    }

    painter.end();
    return lvPath2File.toStdString();
}

std::string CompClusteringEngine::paintHorizontalTree()
{
    QFile file( cvFilePath+"/"+cvFileName+".atr" );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        cvErrorString = "Error when trying to open .atr file";
        return "";
    }

    QTextStream in(&file);
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        QStringList list1 = line.split( "\t",QString::SkipEmptyParts );
        if( cvHTreeNodes.contains( list1.at(1) ) && cvHTreeNodes.contains( list1.at(2) ) )
        {
            QPair<double,double>lvCoords1 = cvHTreeNodes.value( list1.at(1) );
            QPair<double,double>lvCoords2 = cvHTreeNodes.value( list1.at(2) );
            bool lvSuccess;
            double lvBranchLength = 1 -  list1.at(3).toDouble(&lvSuccess) ;
            if( !lvSuccess )
            {
                cvErrorString = "Error when converting to double the correlation value";
                return false;
            }

            double lvCoordX = (lvCoords1.first + lvCoords2.first)/2;
            double lvCoordY = (lvCoords1.second > lvCoords2.second)? lvCoords1.second : lvCoords2.second;
            lvCoordY += lvBranchLength;
            cvHTreeNodes[ list1.at(0) ] = QPair<double,double>( lvCoordX,lvCoordY );
            cvChildNodes[ list1.at(0) ] = QPair<QString,QString>(list1.at(1),list1.at(2));
        }

    }//while

    QImage baseImage( cvDataImgWidth,100,QImage::Format_RGB32 );
    baseImage.fill( 0xFFFFFF );
    QPainter painter( &baseImage );
    QPen highlightedPen( QColor( 125,125,125 ) ); //A beautiful light grey with blue nuances..
    painter.setPen( highlightedPen );

    double lvTotalYSize = 0;
    foreach( QString lvKey, cvChildNodes.keys() )
    {
        lvTotalYSize = cvHTreeNodes[ lvKey ].second > lvTotalYSize ? cvHTreeNodes[ lvKey ].second:lvTotalYSize;
    }

    /*Scaling the size of the tree so that it fits exactly in the x axis*/
    double lvScalingFactorY = 100/lvTotalYSize;

    foreach( QString lvKey, cvChildNodes.keys() )
    {
        double lvCoordX = cvHTreeNodes[ lvKey ].first;
        double lvCoordY = cvHTreeNodes[ lvKey ].second;
        double lvChild1CoordX = cvHTreeNodes[ cvChildNodes[ lvKey ].first ].first;
        double lvChild1CoordY = cvHTreeNodes[ cvChildNodes[ lvKey ].first ].second;
        double lvChild2CoordX = cvHTreeNodes[ cvChildNodes[ lvKey ].second ].first;
        double lvChild2CoordY = cvHTreeNodes[ cvChildNodes[ lvKey ].second ].second;

        lvCoordY *= lvScalingFactorY;
        lvCoordX = cvRectangleWidth/2 + lvCoordX*cvRectangleWidth;
        lvChild1CoordY *= lvScalingFactorY;
        lvChild1CoordX = cvRectangleWidth/2 + lvChild1CoordX*cvRectangleWidth;
        lvChild2CoordY *= lvScalingFactorY;
        lvChild2CoordX = cvRectangleWidth/2 + lvChild2CoordX*cvRectangleWidth;

        lvCoordY = 100 - lvCoordY;
        lvChild1CoordY = 100 - lvChild1CoordY;
        lvChild2CoordY = 100 - lvChild2CoordY;

        painter.drawLine(lvChild1CoordX,lvChild1CoordY,lvChild1CoordX,lvCoordY);
        painter.drawLine(lvChild1CoordX,lvCoordY,lvChild2CoordX,lvCoordY);
        painter.drawLine(lvChild2CoordX,lvCoordY,lvChild2CoordX,lvChild2CoordY);
    }

    // save the result image
    QDate lvCurrentDate = QDate::currentDate();
    QString lvImgsDir = "images"+lvCurrentDate.toString( "yyyyMMdd" );
    QDir lvCurrentDir( cvPath2Images );
    if( ! lvCurrentDir.exists( lvImgsDir ) )
    {
        lvCurrentDir.mkdir( lvImgsDir );
    }
    QString lvFileName = cvFileName+"_h_tree_"+cvTimeStamp+".png";
    QString lvPath2File = lvImgsDir+"/"+lvFileName;
    bool errorCode = baseImage.save( lvCurrentDir.path()+"/"+lvPath2File, "PNG" );
    if(! errorCode)
    {
        cvErrorString = "Error saving horizontal tree image";
        painter.end();
        cvChildNodes.clear();
        return "";
    }

    painter.end();
    cvChildNodes.clear();

    return lvPath2File.toStdString();
}

std::string CompClusteringEngine::paintScale()
{   int x = 0;
    QApplication app(x, 0,false);
    int lvNumPosSquares = 5;
    int lvNumNegSquares = 5;
    const int lvSquareSize = 20;

    if( -cvMinNegValue < cvMaxPosValue )
    {
        lvNumPosSquares = static_cast<int>( (-cvMaxPosValue/cvMinNegValue)*5 );
    }
    else
    {
        lvNumNegSquares = static_cast<int>( (-cvMinNegValue/cvMaxPosValue)*5 );
    }

    QImage baseImage( lvSquareSize*(lvNumPosSquares+1+lvNumNegSquares) + 10,lvSquareSize +30 ,QImage::Format_RGB32 );

    baseImage.fill( 0xFFFFFF );
    QPainter painter( &baseImage );

    //negative number
    for(int i = lvNumNegSquares; i > 0; i--)
    {
        int lvAlpha = i*255/lvNumNegSquares;
        QColor color = QColor(0,0,255, lvAlpha);

        painter.setPen(QColor(0,0,255, 0));
        painter.setBrush(QBrush(color));

        painter.drawRect((lvNumNegSquares-i)*lvSquareSize+5,5,lvSquareSize,lvSquareSize);

    }
    //positive number
    for( int i = 0; i <= lvNumPosSquares; i++ )
    {
        int lvAlpha = i*255/lvNumPosSquares;
        QColor color = QColor(255,0,0, lvAlpha);
        painter.setPen(QColor(255,0,0,0));
        painter.setBrush(QBrush(color));

        painter.drawRect( (i)*lvSquareSize + lvNumNegSquares*lvSquareSize+5, 5, lvSquareSize, lvSquareSize );
    }

    painter.setPen(QColor(0,0,0));
    painter.setBrush(QBrush(QColor(0,0,0)));

    painter.drawLine( 5, 10 + lvSquareSize, 5 + lvSquareSize*(lvNumPosSquares + lvNumNegSquares + 1),  10 + lvSquareSize);
    painter.drawLine( 5, 10 + lvSquareSize, 5, 15 + lvSquareSize );
    painter.drawLine( 5 + lvNumNegSquares*lvSquareSize + lvSquareSize/2, 10 + lvSquareSize, 5 + lvNumNegSquares*lvSquareSize + lvSquareSize/2, 15 + lvSquareSize );
    painter.drawLine( 5 + lvSquareSize*(lvNumPosSquares + lvNumNegSquares + 1), 10 + lvSquareSize, 5 + lvSquareSize*(lvNumPosSquares + lvNumNegSquares + 1), 15 + lvSquareSize );

    QString lvMin;
    lvMin.setNum(this->cvMinNegValue,'g',4);
    QString lvMax;
    lvMax.setNum(this->cvMaxPosValue,'g',4);
    QString lvZero;
    lvZero.setNum(0.0,'g',4);
    QFont lvFont;
    lvFont.setPointSize(9);
    painter.setFont(lvFont);
    QRectF lvRect1(5,15 + lvSquareSize,105,12);
    painter.drawText(lvRect1,Qt::AlignLeft,lvMin);

    QRectF lvRect(5 + lvNumNegSquares*lvSquareSize + lvSquareSize,15 + lvSquareSize,lvSquareSize*(lvNumPosSquares + lvNumNegSquares + 1) - (lvNumNegSquares*lvSquareSize + lvSquareSize),12);
    painter.drawText(lvRect,Qt::AlignRight,lvMax);

    QRectF lvRectCenter(5 + lvNumNegSquares*lvSquareSize,lvSquareSize + 17,lvSquareSize,12); //bug with align center
    painter.drawText(lvRectCenter,Qt::AlignCenter,lvZero);

    // save the result image
    QString lvFileName = cvFileName+"_scale_"+cvTimeStamp+".png";
    QDate lvCurrentDate = QDate::currentDate();
    QString lvImgsDir = "images"+lvCurrentDate.toString( "yyyyMMdd" );
    QDir lvCurrentDir( cvPath2Images );
    QString lvPath2File = lvImgsDir+"/"+lvFileName;
    if( ! lvCurrentDir.exists( lvImgsDir ) )
    {
        lvCurrentDir.mkdir( lvImgsDir );
    }
    bool errorCode = baseImage.save( lvCurrentDir.path()+"/"+lvPath2File, "PNG" );
    if(! errorCode)
    {
        cvErrorString = "Error saving cluster data image";
	painter.end();
	return "";
    }

    painter.end();

    return lvPath2File.toStdString();
}

std::string CompClusteringEngine::paintVerticalTree()
{
    QFile file( cvFilePath+"/"+cvFileName+".gtr" );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        cvErrorString = "Error when trying to open .gtr file";
        return "";
    }

    QTextStream in(&file);
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        QStringList list1 = line.split( "\t",QString::SkipEmptyParts );
        if( cvVTreeNodes.contains( list1.at(1) ) && cvVTreeNodes.contains( list1.at(2) ) )
        {
            QPair<double,double>lvCoords1 = cvVTreeNodes.value( list1.at(1) );
            QPair<double,double>lvCoords2 = cvVTreeNodes.value( list1.at(2) );
            bool lvSuccess;
            double lvBranchLength = 1 - list1.at(3).toDouble(&lvSuccess);
            if( !lvSuccess )
            {
                cvErrorString = "Error when converting to double the correlation value";
                return false;
            }

            double lvCoordX = (lvCoords1.first > lvCoords2.first)? lvCoords1.first : lvCoords2.first;
            lvCoordX += lvBranchLength;
            double lvCoordY = (lvCoords1.second + lvCoords2.second)/2;

            cvVTreeNodes[ list1.at(0) ] = QPair<double,double>( lvCoordX,lvCoordY );
            cvChildNodes[ list1.at(0) ] = QPair<QString,QString>(list1.at(1),list1.at(2));
        }

    }//while

    QImage baseImage( 120,cvDataImgHeight,QImage::Format_RGB32 );
    baseImage.fill( 0xFFFFFF );
    QPainter painter( &baseImage );
    QPen highlightedPen( QColor( 125,125,125 ) );
    painter.setPen( highlightedPen );

    double lvTotalXSize = 0;
    foreach( QString lvKey, cvChildNodes.keys() )
    {
        lvTotalXSize = cvVTreeNodes[ lvKey ].first > lvTotalXSize ? cvVTreeNodes[ lvKey ].first:lvTotalXSize;
    }

    /*Scaling the size of the tree so that it fits exactly in the x axis*/
    double lvScalingFactorX = 120/lvTotalXSize;

    foreach( QString lvKey, cvChildNodes.keys() )
    {
        double lvCoordX = cvVTreeNodes[ lvKey ].first;
        double lvCoordY = cvVTreeNodes[ lvKey ].second;
        double lvChild1CoordX = cvVTreeNodes[ cvChildNodes[ lvKey ].first ].first;
        double lvChild1CoordY = cvVTreeNodes[ cvChildNodes[ lvKey ].first ].second;
        double lvChild2CoordX = cvVTreeNodes[ cvChildNodes[ lvKey ].second ].first;
        double lvChild2CoordY = cvVTreeNodes[ cvChildNodes[ lvKey ].second ].second;

        lvCoordX *= lvScalingFactorX;
        lvCoordY = cvRectangleHeight/2 + lvCoordY*cvRectangleHeight;
        lvChild1CoordX *= lvScalingFactorX;
        lvChild1CoordY = cvRectangleHeight/2 + lvChild1CoordY*cvRectangleHeight;
        lvChild2CoordX *= lvScalingFactorX;
        lvChild2CoordY = cvRectangleHeight/2 + lvChild2CoordY*cvRectangleHeight;

        lvCoordX = 120 - lvCoordX;
        lvChild1CoordX = 120 - lvChild1CoordX;
        lvChild2CoordX = 120 - lvChild2CoordX;

        painter.drawLine(lvCoordX,lvChild1CoordY,lvChild1CoordX,lvChild1CoordY);
        painter.drawLine(lvCoordX,lvChild2CoordY,lvChild2CoordX,lvChild2CoordY);
        painter.drawLine(lvCoordX,lvChild1CoordY,lvCoordX,lvChild2CoordY);

    }

    // save the result image
    QDate lvCurrentDate = QDate::currentDate();
    QString lvImgsDir = "images"+lvCurrentDate.toString( "yyyyMMdd" );
    QDir lvCurrentDir( cvPath2Images );
    if( ! lvCurrentDir.exists( lvImgsDir ) )
    {
        lvCurrentDir.mkdir( lvImgsDir );
    }
    QString lvFileName = cvFileName+"_v_tree_"+cvTimeStamp+".png";
    QString lvPath2File = lvImgsDir+"/"+lvFileName;
    bool errorCode = baseImage.save( lvCurrentDir.path()+"/"+lvPath2File, "PNG" );
    if(! errorCode)
    {
        cvErrorString = "Error saving vertical tree image";
        painter.end();
        cvChildNodes.clear();
        return "";
    }

    painter.end();
    cvChildNodes.clear();

    return lvPath2File.toStdString();
}

std::string CompClusteringEngine::paintCogs( int theYcoordinate, int theHeight )
{
    int x = 0;
    QApplication app(x, 0,false);

    int lvStart = theYcoordinate/cvRectangleHeight;
    int lvFinish = lvStart + theHeight/cvRectangleHeight;

    QFont lvFont;
    lvFont.setPointSize( ZOOMED_SQUARE_SIZE - 2 ); //just to make some space in between lines
    QFontMetrics lvFontMetrics(lvFont);
    const int lvHorizontalSize = lvFontMetrics.width( maxString( cvCogNames ) );

    QImage baseImage( lvHorizontalSize, (lvFinish - lvStart)*ZOOMED_SQUARE_SIZE, QImage::Format_RGB32 );

    baseImage.fill( 0xFFFFFF );
    QPainter painter( &baseImage );
    QPen highlightedPen( QColor( 0,0,0 ) );
    painter.setPen( highlightedPen );

    painter.setBrush(QBrush(QColor( 0,0,0 )));

    painter.setFont( lvFont );

    for( int i = lvStart; i < lvFinish; i++ )
    {
      QRectF lvRect1( 1,(i-lvStart)*ZOOMED_SQUARE_SIZE,lvHorizontalSize,ZOOMED_SQUARE_SIZE );
      painter.drawText( lvRect1,Qt::AlignRight||Qt::AlignVCenter, cvCogNames[i]);
      
    }

    // save the result image
    QString lvFileName = "cogs"+cvTimeStamp+".png";
    QDate lvCurrentDate = QDate::currentDate();
    QString lvImgsDir = "images"+lvCurrentDate.toString( "yyyyMMdd" );
    QDir lvCurrentDir( cvPath2Images );
    if( ! lvCurrentDir.exists( lvImgsDir ) )
    {
        lvCurrentDir.mkdir( lvImgsDir );
    }
    QString lvPath2File = lvImgsDir+"/"+lvFileName;
    bool errorCode = baseImage.save( lvCurrentDir.path()+"/"+lvPath2File, "PNG" );
    if(! errorCode)
    {
        cvErrorString = "Error saving cogs image";
        painter.end();
        return "";
    }

    painter.end();
    return lvPath2File.toStdString();

}

std::string CompClusteringEngine::paintMetagenomes()
{
  int x = 0;
  QApplication app(x, 0,false);


  QFont lvFont;
  lvFont.setPointSize( ZOOMED_SQUARE_SIZE -2 ); //just to make some space in between lines
  QFontMetrics lvFontMetrics(lvFont);
  const int lvVerticalSize = lvFontMetrics.width( maxString( cvMetagenomeNames ) ); 
  QImage baseImage( (cvDataImgWidth/cvRectangleWidth)*ZOOMED_SQUARE_SIZE+1, lvVerticalSize, QImage::Format_RGB32 );

    baseImage.fill( 0xFFFFFF );
    QPainter painter( &baseImage );
    QPen highlightedPen( QColor( 0,0,0 ) );
    painter.setPen( highlightedPen );

    painter.setBrush(QBrush(QColor( 0,0,0 )));

    painter.setFont( lvFont );
    
    for( int i = 0; i < cvMetagenomeNames.size(); i++ )
    {



painter.save();
  //painter.translate(i*ZOOMED_SQUARE_SIZE,lvVerticalSize); 
//painter.rotate(270);
painter.translate((ZOOMED_SQUARE_SIZE*i)+ZOOMED_SQUARE_SIZE,0);  
painter.rotate(90);

    QRectF lvRect1(0,0,lvVerticalSize, ZOOMED_SQUARE_SIZE);
    painter.drawText(lvRect1,Qt::AlignLeft||Qt::AlignVCenter,cvMetagenomeNames[i]);
    //painter.drawText( 0,ZOOMED_SQUARE_SIZE, cvMetagenomeNames[i]);//cvCogNames[i]
   painter.restore();   
    }
    

    // save the result image
    QString lvFileName = "metagenomes"+cvTimeStamp+".png";
    QDate lvCurrentDate = QDate::currentDate();
    QString lvImgsDir = "images"+lvCurrentDate.toString( "yyyyMMdd" );
    QDir lvCurrentDir( cvPath2Images );
    if( ! lvCurrentDir.exists( lvImgsDir ) )
    {
        lvCurrentDir.mkdir( lvImgsDir );
    }
    QString lvPath2File = lvImgsDir+"/"+lvFileName;
    bool errorCode = baseImage.save( lvCurrentDir.path()+"/"+lvPath2File, "PNG" );
    if(! errorCode)
    {
        cvErrorString = "Error saving metagenomes image";
        painter.end();
        return "";
    }

    painter.end();
    return lvPath2File.toStdString();

}

std::string CompClusteringEngine::paintZoomImage( int theYcoordinate, int theHeight )
{
  int x = 0;
  QApplication app(x, 0,false);



    QImage baseImage( (cvDataImgWidth/cvRectangleWidth)*ZOOMED_SQUARE_SIZE+1, (theHeight/cvRectangleHeight)*ZOOMED_SQUARE_SIZE, QImage::Format_RGB32 );

    baseImage.fill( 0xFFFFFF );
    QPainter painter( &baseImage );
    QPen highlightedPen( QColor( 255,255,255 ) );
    painter.setPen(highlightedPen);

    int lvStart = theYcoordinate/cvRectangleHeight;
    int lvFinish = lvStart + theHeight/cvRectangleHeight;

    for( int i = lvStart; i < lvFinish; i++ )
        for( int j = 0; j < this->cvDataMatrix[0].size(); j++ )
        {
	    if(  std::isnan(cvDataMatrix[i][j])  ) //there may be unset values in the cdt file
	    {
	    	QColor color = QColor(170,170,170, 255);
                painter.setBrush(QBrush(color));
                painter.setPen(QColor(170,170,170,0));
	    }
	    else if(cvMinPosValue <= cvDataMatrix[i][j] && cvDataMatrix[i][j] <= cvMaxPosValue)
            {
               //int lvAlpha = 255*((cvDataMatrix[i][j] - cvMinPosValue )/(cvMaxPosValue-cvMinPosValue) );
               int lvAlpha = 255* (cvDataMatrix[i][j]/cvMaxPosValue);
               QColor color = QColor(255,0,0,lvAlpha);
               painter.setBrush(QBrush(color));
               painter.setPen(QColor(255,0,0,0));
            }
            else
            {
               //int lvAlpha = 255*((-cvDataMatrix[i][j] +cvMinPosValue )/(-cvMaxPosValue+cvMinPosValue) );
               int lvAlpha = 255*(cvDataMatrix[i][j]/cvMinNegValue);
               QColor color = QColor(0,0,255, lvAlpha);
               painter.setBrush(QBrush(color));
               painter.setPen(QColor(0,0,255,0));
            }
            painter.drawRect((j*ZOOMED_SQUARE_SIZE),(i-lvStart)*ZOOMED_SQUARE_SIZE,ZOOMED_SQUARE_SIZE,ZOOMED_SQUARE_SIZE);

        }//for

    // save the result image
    QString lvFileName = "zoom"+cvTimeStamp+".png";
    QDate lvCurrentDate = QDate::currentDate();
    QString lvImgsDir = "images"+lvCurrentDate.toString( "yyyyMMdd" );
    QDir lvCurrentDir( cvPath2Images );
    if( ! lvCurrentDir.exists( lvImgsDir ) )
    {
        lvCurrentDir.mkdir( lvImgsDir );
    }
    QString lvPath2File = lvImgsDir+"/"+lvFileName;
    bool errorCode = baseImage.save( lvCurrentDir.path()+"/"+lvPath2File, "PNG" );
    if(! errorCode)
    {
        cvErrorString = "Error saving cluster data image";
        painter.end();
        return "";
    }

    painter.end();
    return lvPath2File.toStdString();
}


bool CompClusteringEngine::readFile( std::string theFileName )
{
    QString lvFileName( theFileName.c_str() );
    if( ! lvFileName.endsWith(".cdt",Qt::CaseInsensitive) ){ return false; }
    QString lvTempString = lvFileName.right( lvFileName.length()-1 - lvFileName.lastIndexOf("/") );
    cvFileName = lvTempString.left( lvTempString.lastIndexOf(".") ); // .cdt
    cvFilePath = lvFileName.left( lvFileName.lastIndexOf("/") );

    QFile file( lvFileName );
    size_t lvLineNumber = 0;
    cvDataMatrix.clear();
    cvCogNames.clear();
    cvMetagenomeNames.clear();

    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ){ return false; }

    cvHTreeNodes.clear();
    cvVTreeNodes.clear();

    QTextStream in(&file);
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        QStringList list1 = line.split("\t",QString::KeepEmptyParts);
        if( 0 == lvLineNumber )
        {
            for( int i = 4; i < list1.size(); i++ ) 
            {
                cvMetagenomeNames.push_back( list1.at(i) );
            }
        }
        if( 1 == lvLineNumber )
        {
            for( int i = 4; i < list1.size(); i++) 
            {
                /*Filling the QMap that contains the coordinates of all samples (metagenomes)*/
                cvHTreeNodes[list1.at(i)] = QPair<double,double>( i-4,0 );
            }
        }
        if( lvLineNumber > 2 )
        {
            /*Filling the QMap that contains the coordinates of all genes (COG's)*/
            cvVTreeNodes[list1.at(0)] = QPair<double,double>( 0,lvLineNumber-3 );

            cvCogNames.push_back( list1.at(1) );

            QVector<double> lvRowData;
            for( int i = 4; i < list1.size(); i++)
            {
                QString lvValueString = list1.at( i );
                bool lvOk;
                double lvValue =lvValueString.toDouble( &lvOk );
                if( !lvOk )
                {
                    if( lvValueString == "" )
                    {
                        lvRowData.push_back( std::numeric_limits<double>::quiet_NaN() );
                    }
                    else
                    {
                        qDebug("Error parsing element");
                        return false;
                    }

                }
                else
                {
                    lvRowData.push_back( lvValue );
                }

                //lvRowData.push_back( lvValue );
            }//for
            cvDataMatrix.push_back( lvRowData );
        }
        lvLineNumber++;
    }
    setMaxMinValues();
    calculateSizes();
    return true;
}

void CompClusteringEngine::calculateSizes()
{
    double lvDataSizeX = this->cvDataMatrix.at(0).size();
    double lvDataSizeY = this->cvDataMatrix.size();

    if( lvDataSizeX > 90)
    {
      this->cvRectangleWidth = 3; 
    }
    else
    {
        cvRectangleWidth = static_cast<int>( 270/lvDataSizeX );
    }

    if( lvDataSizeY > 4500 )
    {
      this->cvRectangleHeight = 3; 
    }
    else
    {
      cvRectangleHeight = 3 + static_cast<int>( 7*(4500 -lvDataSizeY)/(4500-10) );
      cvRectangleHeight = (cvRectangleHeight > 10) ? 10 : cvRectangleHeight;
    }

    cvDataImgWidth = (this->cvDataMatrix.at(0).size()*(cvRectangleWidth) ) + 1;
    cvDataImgHeight = (this->cvDataMatrix.size()*(cvRectangleHeight) ) + 1;
}

QString CompClusteringEngine::maxString( QVector<QString> theVector )
{
  int lvMaxSize = 0;
  QString lvMaxString;

  for( int i = 0; i < theVector.size(); i++ )
  {
    if( theVector[i].size() > lvMaxSize)
    {
      lvMaxSize = theVector[i].size(); 
      lvMaxString = theVector[i];
    }
  }
  
  return lvMaxString;
}

void CompClusteringEngine::setMaxMinValues()
{
    this->cvMaxPosValue = 0;
    this->cvMinPosValue = std::numeric_limits<double>::max( );
    this->cvMaxNegValue = -1*std::numeric_limits<double>::max( );
    this->cvMinNegValue = -1*std::numeric_limits<double>::min( );

    for( int i = 0; i < this->cvDataMatrix.size(); i++ )
        for( int j = 0; j < this->cvDataMatrix[0].size(); j++)
        {
            if( cvDataMatrix[i][j] > cvMaxPosValue && cvDataMatrix[i][j] >= 0 )
            {
                   cvMaxPosValue = cvDataMatrix[i][j];
            }
            if( cvDataMatrix[i][j] < cvMinPosValue && cvDataMatrix[i][j] >= 0 )
            {
                   cvMinPosValue = cvDataMatrix[i][j];
            }
            if( cvDataMatrix[i][j] > cvMaxNegValue && cvDataMatrix[i][j] < 0 )
            {
                   cvMaxNegValue = cvDataMatrix[i][j];
            }
            if( cvDataMatrix[i][j] < cvMinNegValue && cvDataMatrix[i][j] < 0 )
            {
                   cvMinNegValue = cvDataMatrix[i][j];
            }
        }//for
}



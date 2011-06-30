%module compclusteringengine
%include stl.i
%{
#include "compclusteringengine.h"
%}


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

    std::string paintZoomImage( int, int );

    bool readFile( std::string theFileName );


};


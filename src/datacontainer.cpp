/***************************************************************************
 *   Copyright (C) 2006-2011 by the resistivity.net development team       *
 *   Carsten Rücker carsten@resistivity.net                                *
 *   Thomas Günther thomas@resistivity.net                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "datacontainer.h"
#include "pos.h"
#include "vectortemplates.h"

namespace GIMLI{

DataContainer::DataContainer( ){
    initDefaults();
}

DataContainer::DataContainer( const std::string & fileName ){
    initDefaults();
    this->load( fileName );
}

DataContainer::~DataContainer( ){
    initDefaults();
    clear();
}

DataContainer::DataContainer( const DataContainer & data ){
    initDefaults();
    this->copy_( data );
}

DataContainer & DataContainer::operator = ( const DataContainer & data ){
    if ( this != & data ){
        this->copy_( data );
    }
    return * this;
}

void DataContainer::initDefaults(){
    dataMap_["valid"] = RVector( 0 );    
    sensorIndexOnFileFromOne_ = false;
    init();
    initTokenTranslator();
}

void DataContainer::init(){

}

void DataContainer::initTokenTranslator(){
    std::map< std::string, std::string > l;
}

bool DataContainer::haveTranslationForAlias( const std::string & alias ) const {
    std::string test( alias );
    test[ 0 ] = tolower( test[ 0 ] );
    return tT_.count( test );
}

std::string DataContainer::translateAlias( const std::string & alias ) const {
    std::string test( alias );
    test[ 0 ] = tolower( test[ 0 ] );
    if ( tT_.find( test ) != tT_.end() )
        return tT_.find( test )->second;
    return "no alias";
}

void DataContainer::clear( ){
    dataMap_.clear();
}

void DataContainer::copy_( const DataContainer & data ){
    clear();

    topoPoints_ = data.additionalPoints();

    uint nData = data.size();

    this->resize( nData );

    inputFormatString_ = data.inputFormatString();

    dataMap_ = data.dataMap();
}

long DataContainer::createSensor( const RVector3 & pos, double tolerance ){

    long ret = -1;
    for ( uint i = 0; i < sensorPoints_.size(); i ++ ){
        if ( pos.distance( sensorPoints_[ i ] ) < tolerance ){
            ret = i;
        }
    }

    if ( ret == -1 ){
        ret = sensorPoints_.size();
        sensorPoints_.push_back( pos );
    }
    return ret;
}

void DataContainer::registerSensorIndex( const std::string & token ) { 
    dataSensorIdx_.insert( token );
    this->set( token, RVector( this->size(), -1.0 ) );
}

bool DataContainer::isSensorIndex( const std::string & token ) const {
    return dataSensorIdx_.find( token ) != dataSensorIdx_.end();
}

int DataContainer::load( const std::string & fileName ){
    std::fstream file; if ( !openInFile( fileName, & file, true ) );
 
    std::vector < std::string > row; row = getNonEmptyRow( file );
    if ( row.size() != 1 ) {
        std::stringstream str;
        str << WHERE_AM_I << " cannot determine data format. " << row.size() << std::endl;
        throwError( EXIT_DATACONTAINER_NELECS, str.str() );
    }

    //** read number of electrodes
    int nSensors = toInt( row[ 0 ] );
    if ( nSensors < 1 ){
        throwError( EXIT_DATACONTAINER_NELECS, " cannot determine sensor count " + row[ 0 ] );
    }
    RVector x( nSensors, 0.0 ), y( nSensors, 0.0 ), z( nSensors, 0.0 );

    //** read electrodes format
    //** if no electrodes format is given (no x after comment symbol) take defaults
    std::string sensorFormatDefault( "x y z" );
    std::vector < std::string > format( getSubstrings( sensorFormatDefault ) );

    char c; file.get( c );
    if ( c == '#' ) {
        format = getRowSubstrings( file );
//         if ( format[ 0 ][ 0 ] != 'x' && format[ 0 ][ 0 ] != 'X' ){
//             format = getSubstrings( elecsFormatDefault );
//         }
    } else {
        format = getSubstrings( sensorFormatDefault );
    }
    file.unget();

    inputFormatStringSensors_.clear();
    for ( uint i = 0; i < format.size(); i ++ ) inputFormatStringSensors_ += format[ i ] + " ";

    //** read electrodes
    for ( int i = 0; i < nSensors; i ++ ){
        row = getNonEmptyRow( file );
        if ( row.empty() ) {
            std::stringstream str;
            str << WHERE_AM_I << "To few sensor data. " << nSensors << " Sensors expected but " << i << " found." << std::endl;
            throwError( EXIT_DATACONTAINER_NELECS, str.str() );
        }

        for ( uint j = 0; j < row.size(); j ++ ){

            if ( j == format.size() ) break; // no or to few format defined, ignore

            if ( format[ j ] == "x" || format[ j ] == "X" ) x[ i ] = toDouble( row[ j ] );
            else if ( format[ j ] == "x/mm" || format[ j ] == "X/mm" ) x[ i ] = toDouble( row[ j ] ) / 1000.0;
            else if ( format[ j ] == "y" || format[ j ] == "Y" ) y[ i ] = toDouble( row[ j ] );
            else if ( format[ j ] == "y/mm" || format[ j ] == "Y/mm" ) y[ i ] = toDouble( row[ j ] ) / 1000.0;
            else if ( format[ j ] == "z" || format[ j ] == "Z" ) z[ i ] = toDouble( row[ j ] );
            else if ( format[ j ] == "z/mm" || format[ j ] == "z/mm" ) z[ i ] = toDouble( row[ j ] ) / 1000.0;
            else {
                std::stringstream str;
                str << WHERE_AM_I << " Warning! format description unknown: format[ " << j << " ] = " << format[ j ] << " column ignored." << std::endl;
                std::cerr << str.str() << std::endl;
                //throwError( EXIT_DATACONTAINER_ELECS_TOKEN, str.str() );
            }
        }
    }

    for ( int i = 0; i < nSensors; i ++ ) {
        createSensor( RVector3( x[ i ], y[ i ], z[ i ] ).round( 1e-12 ) );
    }
    
    //****************************** Start read the data;
    row = getNonEmptyRow( file );
    if ( row.size() != 1 ) {
        std::stringstream str;
        str << WHERE_AM_I << " cannot determine data size. " << row.size() << std::endl;
        throwError( EXIT_DATACONTAINER_NELECS, str.str() );
    }

    int nData = toInt( row[ 0 ] );

    //bool fromOne    = true;
    bool schemeOnly = false;

    if ( nData > 0 ){
        this->resize( nData );

        //** looking for # symbol which start format description section
        file.get( c );
        if ( c == '#' ) {
            format = getRowSubstrings( file );
            if ( format.size() == 0 ) {
                std::stringstream str;
                str << WHERE_AM_I << "Can not determine data format." << std::endl;
                throwError( EXIT_DATACONTAINER_NO_DATAFORMAT, str.str() );
            }
            if ( format.size() == 4 ) schemeOnly = true;
        }
        file.unget();

        inputFormatString_.clear();
        for ( uint i = 0; i < format.size(); i ++) inputFormatString_ += format[ i ] + " ";
    }

    std::map< std::string, RVector > tmpMap;

    for ( int data = 0; data < nData; data ++ ){
        row = getNonEmptyRow( file );

        if ( row.empty() ) {
            std::stringstream str;
            str << WHERE_AM_I << " To few data. " << nData << " data expected and "
                    << data << " data found." << std::endl;
            throwError( EXIT_DATACONTAINER_DATASIZE, str.str() );
        }

// should only used in speccialication
// if ( row.size() == 4 ) schemeOnly = true;
//        int a = -1, b = -1, m = -1, n = -1;

        for ( uint j = 0; j < row.size(); j ++ ){
            if ( j == format.size() ) break;

            if ( !tmpMap.count( format[ j ] ) ){
                tmpMap.insert( std::pair< std::string, RVector > ( format[ j ], RVector( nData, 0.0 ) ) ) ;
            }
// 	    std::cout << format[ j ] << " " << tmpMap[ format[ j ] ].size() << std::endl;
// 	    std::cout << row[j] << " " << toDouble( row[ j ] ) << std::endl;
            tmpMap[ format[ j ] ][ data ] = toDouble( row[ j ] );
        }
    }

    //** renaming formats with the token translator (tT_):
    for ( std::map< std::string, RVector >::iterator it = tmpMap.begin(); it != tmpMap.end(); it++ ){
        if ( haveTranslationForAlias( it->first ) ){

            double scale = 1.0;
 // should only used in speccialication
            if ( it->first.rfind( "mV" ) != std::string::npos ||
                it->first.rfind( "mA" ) != std::string::npos ||
                it->first.rfind( "ms" ) != std::string::npos ){
                scale = 1.0 / 1000.0;
            }
 
// 	    std::cout << min( it->second ) <<  " " << max( it->second ) << std::endl;

	    //std::cout << "Insert translated:-" << tT_[ it->first ] << "-" << it->second.size() << std::endl;
            dataMap_[ translateAlias( it->first ) ] = it->second * scale;
        } else {
            //std::cout << "Insert:-" <<  it->first << "-" << it->second.size() << std::endl;
            dataMap_[ it->first ] = it->second;
        }
    }

    if ( sensorIndexOnFileFromOne_ ){
        for ( std::map< std::string, RVector >::iterator it = dataMap_.begin(); it!= dataMap_.end(); it ++ ){
            if ( isSensorIndex( it->first ) ){
                it->second -= RVector( size(), 1. );
            }
        }
    }
    
    
    dataMap_[ "valid" ] = 1;

    // validity check should only used in speccialication
    this->checkDataValidity( );
    
    //** start read topo;
    row = getNonEmptyRow( file );
    if ( row.size() == 1 ) {
        //** we found topo
        nSensors = toInt( row[ 0 ] );
        RVector xt( nSensors ), yt( nSensors ), zt( nSensors );

//** if no electrodes format is given (no x after comment symbol) take defaults;
//     format = getSubstrings( elecsFormatDefault );

//     file.get( c );
//     if ( c == '#' ) {
//       format = getRowSubstrings( file );
//       if ( format[ 0 ] != "x" && format[ 0 ] != "X" ){
// 	format = getSubstrings( elecsFormatDefault );
//       }
//     }
//     file.unget();

    //** read topopoints;
        for ( int i = 0; i < nSensors; i ++ ){
            row = getNonEmptyRow( file );

            if ( row.empty() ) {
                std::stringstream str;
                str << WHERE_AM_I << "To few topo data. " << nSensors
                        << " Topopoints expected and " << i << " found." << std::endl;
                throwError( EXIT_DATACONTAINER_NTOPO, str.str() );
            }

            for ( uint j = 0; j < row.size(); j ++ ){
                if ( j == format.size() ) break; // no or to few format defined, ignore

                if (      format[ j ] == "x" || format[ j ] == "X" ) xt[ i ] = toDouble( row[ j ] );
                else if ( format[ j ] == "y" || format[ j ] == "Y" ) yt[ i ] = toDouble( row[ j ] );
                else if ( format[ j ] == "z" || format[ j ] == "Z" ) zt[ i ] = toDouble( row[ j ] );
                else {
                    std::stringstream str;
                    str << " Warning! format description unknown: topo electrode format[ "
                        << j << " ] = " << format[ j ] << ". column ignored." << std::endl;
                    throwError( EXIT_DATACONTAINER_ELECS_TOKEN, str.str() );
                }
            }
        }

        for ( int i = 0 ; i < nSensors; i ++ ) {
            topoPoints_.push_back( RVector3( xt[ i ], yt[ i ], zt[ i ] ).round( 1e-12 ) );
        }
    } // if topo

    file.close();
    return 1;
}

void DataContainer::checkDataValidity( bool remove ){
    //** mark inf and nans as invalid
    for ( std::map< std::string, RVector >::iterator it = dataMap_.begin();
            it!= dataMap_.end(); it ++ ){
        this->markInvalid( find( isInfNaN( it->second ) ) );
    }
    
    //** call local specialization if any
    checkDataValidityLocal();

    if ( find( get("valid") < 1 ).size() > 0 ){
        std::cout << "Data validity check: found " << find( get("valid") < 1 ).size() << " invalid data. " << std::endl;
        if ( remove ) {
            std::cout << "Data validity check: remove invalid data." << std::endl;
            this->removeInvalid();
        }
    }
}

int DataContainer::save( const std::string & fileName, const std::string & formatData,
                         const std::string & formatSensor, bool verbose ) const {
    
    std::fstream file; if ( !openOutFile( fileName, & file ) ) return 0;

    file.precision( 14 );
    
    //** START write sensor data
    file << sensorPoints_.size() << std::endl;
    std::string sensorsString( formatSensor );
    file << "# " << sensorsString << std::endl;

    std::vector < std::string > format( getSubstrings( sensorsString ) );
    for ( uint i = 0, imax = sensorPoints_.size(); i < imax; i ++ ){
        for ( uint j = 0; j < format.size(); j ++ ){
            if ( format[ j ] == "x" || format[ j ] == "X" ) file << sensorPoints_[ i ].x();
            if ( format[ j ] == "y" || format[ j ] == "Y" ) file << sensorPoints_[ i ].y();
            if ( format[ j ] == "z" || format[ j ] == "Z" ) file << sensorPoints_[ i ].z();
            if ( j < format.size() -1 ) file << "\t";
        }
        file << std::endl;
    }

    //** START write data map
    std::vector < const RVector * > outVec;
    std::vector < bool > outInt;
    
    IndexArray toSaveIdx;
    std::string formatString;
    
    if ( lower( formatData ) == "all" ){
        toSaveIdx = find( get("valid") > -1 );
        formatString = this->tokenList();
    } else {
        toSaveIdx = find( get("valid") == 1 );
        formatString = formatData;
    }
    
    int count = toSaveIdx.size();
    file << count << std::endl;
    file << "# " << formatString << std::endl;

    std::vector < std::string > token( getSubstrings( formatString ) );

    for ( uint i = 0; i < token.size(); i ++ ){
        //  std::cout << token[ i ] << std::endl;

        std::string valName;
        if ( haveTranslationForAlias( token[ i ] ) ){
            valName = translateAlias( token[ i ] );
        } else {
            valName = token[ i ];
        }

        if ( dataMap_.count( valName ) ){
            outVec.push_back( &dataMap_.find( valName )->second  );

            if ( isSensorIndex( valName ) ){
                outInt.push_back( true );
            } else {
                outInt.push_back( false );
            }
        } else {
            throwError( 1, WHERE_AM_I + " no such data: " + valName );
        }
    }

    for ( uint i = 0; i < toSaveIdx.size(); i ++ ){
        file.setf( std::ios::scientific, std::ios::floatfield );
        file.precision( 14 );

        for ( uint j = 0; j < token.size(); j ++ ){

            if ( outInt[ j ] ){
                file << int( ( *outVec[ j ] )[ toSaveIdx[ i ] ] ) + sensorIndexOnFileFromOne_;
            } else {
                 if ( token[ j ] == "valid" ){
                    file << int( ( *outVec[ j ] )[ toSaveIdx[ i ] ] );
                 } else {
                    file << ( *outVec[ j ] )[ toSaveIdx[ i ] ];
                 }
            }
            if ( j < token.size() -1 ) file << "\t";
        }
        file << std::endl;
    }
    
    //** START write additional points
    file << topoPoints_.size() << std::endl;
    for ( uint i = 0; i < topoPoints_.size(); i ++ ){
        std::cout   << topoPoints_[ i ].x() << "\t" 
                    << topoPoints_[ i ].y() << "\t" 
                    << topoPoints_[ i ].z() << std::endl;
    }
        
    file.close();

    if ( verbose ){
        std::cout << "Wrote: " << fileName << " with " << sensorPoints_.size()
                << " sensors and " << count << " data." <<  std::endl;
    }
    return 1;
}

std::string DataContainer::tokenList() const {
    std::string tokenList;
    for ( std::map< std::string, RVector >::const_iterator it = dataMap_.begin(); it!= dataMap_.end(); it ++ ){
        tokenList += it->first;
        tokenList += " ";
    }
    return tokenList;
}

void DataContainer::showInfos() const {
    std::cout << "Sensors: " << this->sensorCount() << ", Data: " << this->size();
    if ( topoPoints_.size() > 0 ){
        std::cout << " Topopoints: " << topoPoints_.size();
    }
    std::cout << std::endl << tokenList() << std::endl;
}

void DataContainer::add( const std::string & token, const RVector & data, const std::string & description ){
    this->set( token, data );
    this->setDataDescription( token, description );
//     if ( data.size() == this->size() ) {
//         dataMap_.insert( make_pair( token, data ) );
//         this->setDataDescription( token, description );
//     } else {
//         throwError( 1, WHERE_AM_I + " wrong data size: " + toStr( this->size() ) + " " + toStr( data.size() ) );
//     }
}

void DataContainer::set( const std::string & token, const RVector & data ){
    if ( data.size() == this->size() ){
        dataMap_[ token ] = data;
    } else {
        throwError( 1, WHERE_AM_I + " wrong data size: " + toStr( this->size() ) + " " + toStr( data.size() ) );
    }
}

const RVector & DataContainer::get( const std::string & token ) const {
    if ( dataMap_.count( token ) ) {
        return dataMap_.find( token )->second;
    }

    throwError( 1, WHERE_AM_I + " unknown token data for get: " + token + " available are: " + tokenList()  );
    return *new RVector( 0 );
}

RVector * DataContainer::ref( const std::string & token ){
    if ( dataMap_.count( token ) ) {
        return &dataMap_.find( token )->second;
    }

    throwError( 1, WHERE_AM_I + " unknown token data for ref: " + token + " available are: " + tokenList()  );
    return NULL;
}

void DataContainer::setDataDescription( const std::string & token, const std::string & description ){
    std::cout << "( this->exists( token ) ) " << this->exists( token ) << std::endl;
    if ( this->exists( token ) ){
        dataDescription_[ token ] = description;
    }
}

std::string DataContainer::dataDescription( const std::string & token ) const {
    if ( this->exists( token ) && ( dataDescription_.count( token ) ) ) {
        return dataDescription_.find( token )->second;
    }
    return "";
}

void DataContainer::resize( uint size ) {
    for ( std::map< std::string, RVector >::iterator it = dataMap_.begin();
            it!= dataMap_.end(); it ++ ){
        
        if ( isSensorIndex( it->first ) ){
            // if the data field represent a sensorIDX fill with -1
            it->second.resize( size, -1.0 );
        } else {
            // else pure data, fill with 0.0
            it->second.resize( size, 0.0 );
        }
        
    }
}

void DataContainer::removeInvalid(){
    std::vector< size_t > validIdx( find( get("valid") == 1 ) );

    for ( std::map< std::string, RVector >::iterator it = dataMap_.begin(); it!= dataMap_.end(); it ++ ){
        dataMap_[ it->first ] = it->second( validIdx );
    }
}

void DataContainer::remove( const IndexArray & idx ){
    this->markValid( idx, false );
    this->removeInvalid();
}

DataContainer DataContainer::filter( const IndexArray & idx ) const {
    DataContainer data( *this );
    data.markValid( find( data("valid") > -1 ), false );
    data.markValid( idx, true );
    data.removeInvalid();
    return data;
}

// START Sensor related section
void DataContainer::removeSensorIdx( uint idx ){
    IndexArray i(1, idx );
    this->removeSensorIdx( i );
}
    
void DataContainer::removeSensorIdx( const IndexArray & idx ){
    for ( std::map< std::string, RVector >::iterator it = dataMap_.begin(); it!= dataMap_.end(); it ++ ){
        if ( isSensorIndex( it->first ) ){
            for ( IndexArray::const_iterator id = idx.begin(); id != idx.end(); id ++ ){
                this->markValid( find( it->second == *id ), false );
            }
        }
    }
    this->removeInvalid();
    this->removeUnusedSensors();
}
    
void DataContainer::removeUnusedSensors(){
    
    BVector activeSensors( this->sensorCount(), false );
    
    for ( std::map< std::string, RVector >::iterator it = dataMap_.begin(); it!= dataMap_.end(); it ++ ){
        if ( isSensorIndex( it->first ) ){
            for ( uint i = 0; i < it->second.size(); i ++ ){
                ssize_t id = it->second[ i ];
                if ( id > -1 && id < (ssize_t)this->sensorCount() ) activeSensors[ id ] = true;
            }
        }
    }
    
    IndexArray perm( this->sensorCount(), 0 );

    std::vector < RVector3 > tmp( sensorPoints_ );
    sensorPoints_.clear();

    for ( size_t i = 0; i < activeSensors.size(); i ++ ) {
        if ( activeSensors[ i ] ){
            sensorPoints_.push_back( tmp[ i ] );
        }
        perm[ i ] = sensorPoints_.size() -1;
    }

    for ( std::map< std::string, RVector >::iterator it = dataMap_.begin(); it!= dataMap_.end(); it ++ ){
        if ( isSensorIndex( it->first ) ){
            for ( uint i = 0; i < it->second.size(); i ++ ){
                ssize_t id = it->second[ i ];
                if ( id > -1 && id < (ssize_t)perm.size() ) it->second[ i ] = perm[ id ];
            }
        }
    }    
}

void DataContainer::markInvalidSensorIndices(){
    for ( std::map< std::string, RVector >::iterator it = dataMap_.begin(); it!= dataMap_.end(); it ++ ){
        if ( isSensorIndex( it->first ) ){
            this->markValid( find( it->second >= this->sensorCount() ), false );
        }
    }
}
// END Sensor related section

} // namespace GIMLI{

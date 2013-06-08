/*
 * Program: NagPerfDiag
 * License: GNU/GPL
 * Copyright (c) 2009 Herbert Straub <herbert@linuxhacker.at>
 * Website: www.linuxhacker.at
 *
 * License:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "rrdmanager.h"
#include "performancedata.h"

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <rrd.h>
#include <string>
#include <sys/stat.h>
#include <vector>

cRrdManager::cRrdManager( )
        : rrdDirectory( "/tmp/testrrd" ),
        dirMode( S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP ),
        rrdMode( S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP )
{
}

void cRrdManager::checkRrdsAndCreateIfNotAvailable( cNpdData &inputData )
{
    std::vector<spServicePerfData>::iterator iter;
    mcRrdFilenameToPerformanceValueNameIter valueRrdIter;
    int createFlag;

    this->mRrdFilenameCache = mcRrdFilenameCache( );

    for ( iter = inputData.performanceVector.begin( );
            iter != inputData.performanceVector.end( );
            iter++ ) {
        this->buildRrdNameComponentsFromPerformanceData( *iter );

        for ( createFlag = 0,
                valueRrdIter = (*iter)->rrdNameComponents.mRrdFilenameToPerformanceValueName.begin( );
                valueRrdIter != (*iter)->rrdNameComponents.mRrdFilenameToPerformanceValueName.end( );
                valueRrdIter++) {
            if ( this->mRrdFilenameCache.count( valueRrdIter->first ) > 0 ) {
                continue;
            }
            if ( ! isDirectoryAvailable( valueRrdIter->first ) ) {
                createFlag = 1;
            } else {
                this->mRrdFilenameCache[valueRrdIter->first] = 1;
            }
        }

        if ( createFlag == 0 ) {
            continue;
        }
        if ( ! isDirectoryAvailable( (*iter)->rrdNameComponents.baseDirectory ) ) {
            createDirectory( (*iter)->rrdNameComponents.baseDirectory );
        }
        if ( ! isDirectoryAvailable( (*iter)->rrdNameComponents.hostDirectory ) ) {
            createDirectory( (*iter)->rrdNameComponents.hostDirectory );
        }
        if ( ! isDirectoryAvailable( (*iter)->rrdNameComponents.serviceDirectory ) ) {
            createDirectory( (*iter)->rrdNameComponents.serviceDirectory );
        }

        for ( valueRrdIter = (*iter)->rrdNameComponents.mRrdFilenameToPerformanceValueName.begin( );
                valueRrdIter != (*iter)->rrdNameComponents.mRrdFilenameToPerformanceValueName.end( );
                valueRrdIter++) {
            if ( ! isDirectoryAvailable( valueRrdIter->first ) ) {
                this->createSingleRrd( valueRrdIter->first, *iter,
                                       valueRrdIter->second);
                this->mRrdFilenameCache[valueRrdIter->first] = 1;
            }
        }
    }
}

void cRrdManager::updateRrdsWithPerformanceData( cNpdData &inputData )
{
    std::vector<spServicePerfData>::iterator iter;
    mcRrdFilenameToPerformanceValueNameIter valueRrdIter;
    std::string updateRrdString;
    int status;

    for ( iter = inputData.performanceVector.begin( );
            iter != inputData.performanceVector.end( );
            iter++ ) {
        for ( valueRrdIter = (*iter)->rrdNameComponents.mRrdFilenameToPerformanceValueName.begin( );
            valueRrdIter != (*iter)->rrdNameComponents.mRrdFilenameToPerformanceValueName.end( );
            valueRrdIter++) {
            // wir brauchen rrdfilename, ds (==value), PerformanceVector (timestamp:value)
            std::vector<std::string> vDsData;
            std::string dsData;
            dsData = (*iter)->mNagiosData["timet"];
            dsData += ":";
            dsData += (*iter)->mValueToPerformanceData[valueRrdIter->second].value;
            vDsData.push_back( dsData );
            updateRrdString = dsData;

            status = this->rrd_update( valueRrdIter->first, valueRrdIter->second, vDsData );
            if (status != 0 ) {
                (*iter)->rrdOperationsLogger.logFailure( valueRrdIter->first,
                    valueRrdIter->second,
                    std::string( "udpate" ), updateRrdString, this->rrdErrorText );
            } else {
                (*iter)->rrdOperationsLogger.logSuccess( valueRrdIter->first,
                    valueRrdIter->second,
                    std::string( "update " ), updateRrdString );
            }
        }
    }
}

// TODO: move to npdrrdnameconverter
void cRrdManager::buildRrdNameComponentsFromPerformanceData( spServicePerfData pServiceItem )
{

    pServiceItem->rrdNameComponents.baseDirectory = this->rrdDirectory;
    pServiceItem->rrdNameComponents.hostDirectory = this->rrdDirectory + "/"
            + pServiceItem->hostname;
    pServiceItem->rrdNameComponents.serviceDirectory = pServiceItem->rrdNameComponents.hostDirectory
            + "/" + pServiceItem->servicedesc;
    mcValueToPerformanceDataIter valueIter;
    for ( valueIter = pServiceItem->mValueToPerformanceData.begin( );
            valueIter != pServiceItem->mValueToPerformanceData.end( );
            valueIter++) {
        pServiceItem->rrdNameComponents.mRrdFilenameToPerformanceValueName
        [pServiceItem->rrdNameComponents.serviceDirectory + "/" + valueIter->first + ".rrd"]
        = valueIter->first;
    }
}

void cRrdManager::createSingleRrd( const std::string rrdName, spServicePerfData pServicePerfData,
                                   std::string valueName )
{
    mcValueToPerformanceDataIter valueIter;
    std::vector<std::string> vDsStrings;
    std::string timet = pServicePerfData->mNagiosData["timet"];
    std::string rrdDataType;
    std::string rrdMin = "U";
    std::string rrdMax = "U";
    std::string rrdDsString;
    int status;

    if ( pServicePerfData->mValueToPerformanceData[valueName].min != "" ) {
        rrdMin = pServicePerfData->mValueToPerformanceData[valueName].min;
    }
    if ( pServicePerfData->mValueToPerformanceData[valueName].max != "" ) {
        rrdMax = pServicePerfData->mValueToPerformanceData[valueName].max;
    }
    rrdDataType = pServicePerfData->mValueToPerformanceData[valueName].rrdDataType;

    rrdDsString = "DS:" + valueName + ":" + rrdDataType + ":1800:" + rrdMin + ":" + rrdMax;
    std::cout << "Das ist der rrdString für " << valueName << " " << rrdDsString << std::endl;
    vDsStrings.push_back( rrdDsString );

    this->fillRraDataInVector( vDsStrings );

    status = this->rrd_create( rrdName, std::string( "60"),timet, vDsStrings );
    if (status != 0 ) {
        pServicePerfData->rrdOperationsLogger.logFailure( rrdName, valueName,
            std::string( "create" ), rrdDsString, this->rrdErrorText );
    } else {
        pServicePerfData->rrdOperationsLogger.logSuccess( rrdName, valueName,
            std::string( "create" ), rrdDsString );
    }
}

int cRrdManager::rrd_create( std::string rrdName, std::string step, std::string start,
                             std::vector<std::string> vDsAndRra)
{
    int status;
    int i;
    size_t size;
    std::vector<std::string>::iterator vDsStringsIterator;

    unsigned long ulStep = boost::lexical_cast<unsigned long>( step );
    time_t tStart = boost::lexical_cast<time_t>( start );
    tStart -= 100; // 100 seconds back

    size = vDsAndRra.size( ) + 1;
    char **pTest = new char *[size];

    for (i = 0, vDsStringsIterator = vDsAndRra.begin( ); vDsStringsIterator != vDsAndRra.end( );
            i++, vDsStringsIterator++) {
        pTest[i] =  (char * ) vDsStringsIterator->c_str( );
    }
    pTest[i] = NULL;

    status = rrd_create_r( rrdName.c_str( ), ulStep, tStart, vDsAndRra.size( ),
                           (const char **) pTest  );

    delete [] pTest;

    if ( status != 0 ) {
        rrdErrorText = rrd_get_error( );
        status = 1;
    } else {
        rrdErrorText = "";
        status = 0;
    }

    return status;
}

int cRrdManager::rrd_update( std::string rrdName, std::string rrdTemplate,
    std::vector<std::string> vRrdUpdateStrings )
{
    size_t size;
    int i;
    int status;
    std::vector<std::string>::iterator vRrdUpdateStringsIter;

    size = vRrdUpdateStrings.size( ) + 1;
    char **pTest = new char *[size];

    for( i = 0, vRrdUpdateStringsIter = vRrdUpdateStrings.begin( );
         vRrdUpdateStringsIter != vRrdUpdateStrings.end( );
         vRrdUpdateStringsIter++, i++ ) {
        pTest[i] = (char *) vRrdUpdateStringsIter->c_str( );
    }
    pTest[i] = NULL;

	status = rrd_update_r( rrdName.c_str( ), rrdTemplate.c_str( ), vRrdUpdateStrings.size( ),
        (const char **) pTest );

    if ( status != 0 ) {
        rrdErrorText = rrd_get_error( );
        status = 1;
    } else {
        rrdErrorText = "";
        status = 0;
    }

    delete [] pTest;

    return status;
}

void cRrdManager::fillRraDataInVector( std::vector<std::string> &vRraData )
{
    // FIXME: Echtwerte
    vRraData.push_back( std::string( "RRA:AVERAGE:0.5:1:2016" ) );
    vRraData.push_back( std::string( "RRA:LAST:0.5:1:2016" ) );
    vRraData.push_back( std::string( "RRA:MAX:0.5:1:2016" ) );
}

int cRrdManager::isDirectoryAvailable( const std::string directoryName )
{
    int status;
    struct stat buf;

    status = stat( directoryName.c_str( ), &buf );
    if ( status == 0 ) {
        return 1;
    } else {
        return 0;
    }
}

void cRrdManager::createDirectory( const std::string directoryName )
{
    int status;

    std::cout << "creating: " << directoryName<< std::endl;
    status = mkdir( directoryName.c_str( ), dirMode );
    std::cout << "mkdir status: " << status << std::endl;
    //FIXME: Error Handling
}

int cRrdManager::isRrdUpdatePossible( const std::string rrdFilename )
{
    return isDirectoryAvailable( rrdFilename );
}
/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

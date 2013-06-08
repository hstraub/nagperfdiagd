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

#include "npdlogger.h"

#include <boost/lexical_cast.hpp>
#include <iostream>

#include <unistd.h>

cNpdLogger::cNpdLogger( spcNpdConfiguration spNpdConfiguration )
    : spNpdConfiguration( spNpdConfiguration ), outputDebugDirectoryIsWriteable( 0 ) 
{
    connectToSql( );
}

void cNpdLogger::connectToSql( void )
{
    std::string sqlConnectionString;

    sqlConnectionString = "dbname=" + spNpdConfiguration->getParameter( "db_databasename" ) + " ";
    sqlConnectionString += "user=" +spNpdConfiguration->getParameter( "db_username" ) + " ";
    sqlConnectionString += "password=" + spNpdConfiguration->getParameter( "db_password" );

    std::cout << "cNpdLogger: Database connection string: " << sqlConnectionString << std::endl;
    sqlSession.open( soci::postgresql , sqlConnectionString );
}

int cNpdLogger::isDebugDirectoryAvailableAndWriteable( void )
{
    /* FIXME: really necessary?
    std::ofstream debugFile;
    std::string filename = spNpdConfiguration->getParameter( "debug_output_directory ")
        + "test.test"; 

    debugFile.open( filename );
    debugFile << "Test Test" << std::endl;
    if ( debugFile.bad( ) ) {
        std::cout << "cNpdLogger: Error, cannot write to output_debug_directory: "
            << filename << std::endl;
        std::cout << "cNpdLogger: Writing to std::cout!" << std::endl;
    }
    debugFile.close( );
    unlink( filename.c_str( ) );
    */
}

void cNpdLogger::writeRrdLogsToDatabase( spcNpdData spNpdData )
{
    vspServicePerfDataIter iter = spNpdData->performanceVector.begin( );
    std::vector<cRrdOperationLog>::iterator iterLogger;

    for ( ; iter != spNpdData->performanceVector.end( ); iter++ ) {
        iterLogger = (*iter)->rrdOperationsLogger.vRrdOperationsLog.begin( );
        for ( ; iterLogger != (*iter)->rrdOperationsLogger.vRrdOperationsLog.end( );
                iterLogger++ ) {
            if ( iterLogger->rrdOperation == "create" ) {
                insertOrUpdateLogs( (*iter)->hostname, (*iter)->servicedesc, 
                    iterLogger->valuename, (*iter)->mNagiosData["timet"],
                    (*iter)->mValueToPerformanceData[iterLogger->valuename].rrdDataType, 
                    (*iter)->servicecheckcommand );
            }
        }
    }
}

void cNpdLogger::insertOrUpdateLogs( const std::string & hostname, const std::string &servicedesc,
        const std::string &valuename, const std::string &timet, const std::string &datatype,
        const std::string &checkcommand )
{
    int i;

    sqlSession.begin( );
    sqlSession << "select count( * ) from rrd_creation_log where "
        << "hostname = :hostname and servicedesc = :servicedesc "
        << "and valuename = :valuename", soci::into( i ), 
        soci::use( hostname, "hostname" ),
        soci::use( servicedesc, "servicedesc" ),
        soci::use( valuename, "valuename" );

    if ( i > 0 ) {
        sqlSession << "update rrd_creation_log set "
            << "hostname = :hostname, "
            << "servicedesc = :servicedesc, "
            << "valuename = :valuename, "
            << "creation_timestamp = :creation_timestamp, "
            << "datatype = :datatype, "
            << "service_checkcommand = :service_checkcommand "
            << "where hostname = :hostname and servicedesc = :servicedesc "
            << "and valuename = :valuename",
            soci::use( hostname, "hostname" ),
            soci::use( servicedesc, "servicedesc" ),
            soci::use( valuename, "valuename" ),
            soci::use( boost::lexical_cast<int>( timet ), "creation_timestamp" ),
            soci::use( datatype, "datatype" ),
            soci::use( checkcommand, "service_checkcommand");
    } else {
        sqlSession << "insert into rrd_creation_log( hostname, servicedesc, "
            << "valuename, creation_timestamp, datatype, service_checkcommand ) "
            << "values ( :hostname, :servicedesc, :valuename, :creation_timestamp, "
            << ":datatype, :service_checkcommand )",
            soci::use( hostname, "hostname" ),
            soci::use( servicedesc, "servicedesc" ),
            soci::use( valuename, "valuename" ),
            soci::use( boost::lexical_cast<int>( timet ), "creation_timestamp" ),
            soci::use( datatype, "datatype" ),
            soci::use( checkcommand, "service_checkcommand");
    }

    sqlSession.commit( );
}

/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

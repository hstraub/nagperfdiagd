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

#include "npdtypecache.h"
#include "npddataconverter.h"

#include <boost/lexical_cast.hpp>
#include <exception>
#include <iostream>
#include <soci/soci.h>
#include <soci/soci-postgresql.h>


cNpdTypeCache::cNpdTypeCache( spcNpdConfiguration spNpdConfiguration )
    : spNpdConfiguration ( spNpdConfiguration )
{
    connectToSql( );
    mCheckcommandsToDataTypeIterNotFound = mCheckcommandsToDataType.end( );
}

void cNpdTypeCache::connectToSql( void )
{
    std::string sqlConnectionString;

    sqlConnectionString = "dbname=" + spNpdConfiguration->getParameter( "db_databasename" ) + " ";
    sqlConnectionString += "user=" +spNpdConfiguration->getParameter( "db_username" ) + " ";
    sqlConnectionString += "password=" + spNpdConfiguration->getParameter( "db_password" );

    std::cout << "Database connection string: " << sqlConnectionString << std::endl;
    sqlSession.open( soci::postgresql , sqlConnectionString );
}

void cNpdTypeCache::getDataTypeFor( const std::string &hostname, const std::string &servicedesc,
        const std::string &checkcommand, const std::string &valueName, std::string &dataType )
{
    std::string hashValue = checkcommand + valueName;
    if ( mCheckcommandsToDataType.find( hashValue ) != mCheckcommandsToDataTypeIterNotFound ) {
        std::cout << "cNpdTypeCache::getDataTypeFor checkcommand:"
            << checkcommand << " valueName:" << valueName 
            << " found in cache" << std::endl;
        dataType = mCheckcommandsToDataType[hashValue];
    } else {
        std::cout << "cNpdTypeCache::getDataTypeFor checkcommand:"
            << checkcommand << " valueName:" << valueName 
            << " not found in cache reading from SQL" << std::endl;
        readFromSql( checkcommand, valueName, dataType );
        mCheckcommandsToDataType[hashValue] = dataType;
    }
}

void cNpdTypeCache::readFromSql ( const std::string &checkcommand,
    const std::string &valueName, std::string &dataType )
{
    boost::regex dataTypeRegex;
    boost::smatch what;
    int foundFlag = 0;
    soci::rowset<soci::row> rowVector = ( sqlSession.prepare 
            << "select checkcommandname, ranking, regular_expression, rrd_datatype "
            << "from data_type_conversion "
            << "where checkcommandname=:name "
            << "order by checkcommandname, ranking ", soci::use( checkcommand, "name" ) );
    
    // FIXME soci error? 
    /*
    if ( rowVector.empty( ) ) {
        std::cout << "rowVector(" << checkcommand <<  ") ist leer!" << std::endl;
    } else {
        std::cout << "rowVector(" << checkcommand <<  ") ist nicht empty" << std::endl;
    }
    */

    soci::rowset<soci::row>::iterator rowVectorIter;
    
    for ( rowVectorIter = rowVector.begin( ); rowVectorIter != rowVector.end( ); rowVectorIter++ ) {
        std::cout << "Row: " << rowVectorIter->get<std::string>( 0 )
            << " ranking: " << rowVectorIter->get<int>( 1 )
            << " regular_expresssion: " << rowVectorIter->get<std::string>( 2 )
            << " rrd_datatype: " << rowVectorIter->get<std::string>( 3 )
            << std::endl;
        dataTypeRegex = rowVectorIter->get<std::string>( 2 );
        std::cout << "we are matching valueName: " << valueName 
            << " with dataTypeRegex:" << dataTypeRegex << std::endl;
        if ( boost::regex_match( valueName, what, dataTypeRegex ) ) {
            std::cout << "yes, this matching: valueName: " << valueName 
                << " with dataTypeRegex:" << dataTypeRegex << std::endl;
            dataType = rowVectorIter->get<std::string>( 3 );
            foundFlag = 1;
            break;
        }
    }

    if ( foundFlag == 0 ) {
        dataType = spNpdConfiguration->getParameter( "rrd_default_datatype" );
    }

    std::cout << "From SQL: for checkcommand: " << checkcommand 
        << " valueName: " << valueName
        << " datatype: " << dataType << std::endl;

}
/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

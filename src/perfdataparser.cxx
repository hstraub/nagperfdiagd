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

#include "perfdataparser.h"

#include <iostream>
#include <boost/tokenizer.hpp>

npdPerfDataParser::npdPerfDataParser( spcNpdConfiguration spNpdConfiguration )
{
    dataRecordRegex = "(\\w+)::(.*)$";
    performanceDataRegex = "'?([^'=]+)'?=([^ $]+)( |$)";
    performanceItemRegex = "'?([^'=]+)'?=(-?\\d+(?:\\.\\d+)?)([a-zA-Z%]*)([^ ]*) ?$";
    servicecheckcommandRegex = "(\\w+)!.*";
    this->spNpdConfiguration = spNpdConfiguration;
}

void npdPerfDataParser::parseDataStream( std::istream &inputDataStream,
        cNpdData &performanceData )
{
    std::string dataLine;
    spNpdDataVector newDataVector( new vNpdData( ) );
    int inDataBlockFlag = 0;

    while ( ! inputDataStream.eof( ) ) {
        std::getline( inputDataStream, dataLine );
        // std::cout << "data: " << dataLine << std::endl;
        if ( dataLine == "[SERVICEPERFDATA-START]" ) {
            spServicePerfData newPerfDataRecord( new cServicePerfData );
            this->actualServicePerfDataRecord = newPerfDataRecord;
            inDataBlockFlag = 1;
        } else if ( dataLine == "[SERVICEPERFDATA-END]") {
            if ( this->actualServicePerfDataRecord
                    ->getNumberOfPerformanceValues( ) > 0 ) {
                performanceData.performanceVector.push_back(
                    this->actualServicePerfDataRecord );
            } else {
                performanceData.performanceVectorWithoutPerformanceValues
                .push_back( this->actualServicePerfDataRecord );
            }
            inDataBlockFlag = 0;
        } else if ( inDataBlockFlag ) {
            this->parseDataRecord( dataLine );
        }
    }
}

void npdPerfDataParser::parseDataRecord( const std::string &dataRecord )
{
    boost::smatch what;

    if ( boost::regex_match( dataRecord, what, this->dataRecordRegex,
                             boost::match_default ) ) {
        this->actualServicePerfDataRecord->mNagiosData[what[1]] = what[2];
        if ( what[1] == "serviceperfdata" ) {
            this->parseServicePerfDataRecord( what[2] );
        } else if ( what[1] == "hostname" ) {
            this->actualServicePerfDataRecord->hostname = what[2];
        } else if ( what[1] == "servicedesc" ) {
            this->actualServicePerfDataRecord->servicedesc = what[2];
        } else if ( what[1] == "servicecheckcommand" ) {
            this->parseServicecheckcommand( what[2],
                                            this->actualServicePerfDataRecord->servicecheckcommand );
        }
    } else {
        // FIXME: what here
        std::cout << "Fehlerhafter Record: " << dataRecord << std::endl;
    }
}

void npdPerfDataParser::parseServicecheckcommand( const std::string &servicecheckcommand,
        std::string &parsedServicecheckcommand )
{
    boost::smatch what;

    if ( boost::regex_match( servicecheckcommand, what,
                             this->servicecheckcommandRegex ) ) {
        parsedServicecheckcommand = what[1];
    } else {
        // FIXME: what here?
        //this->parsePerformanceValueItemSuccessfullFlag = 0;
    }
}

void npdPerfDataParser::parseServicePerfDataRecord( const std::string &servicePerfData )
{
    boost::smatch what;
    std::vector<std::string> matchingValues;

    // std::cout << "analysing1: " << servicePerfData << std::endl;

    boost::sregex_token_iterator i(
        servicePerfData.begin(),
        servicePerfData.end(),
        this->performanceDataRegex);
    boost::sregex_token_iterator j;


    while ( i != j ) {
        cPerformanceData data;
        this->parsePerformanceValueItem( *i, data );
        if ( this->parsePerformanceValueItemSuccessfullFlag ) {
            this->actualServicePerfDataRecord->mValueToPerformanceData[data.name]
            = data;
        } else {
            // Fixme: what here?
            std::cout << "Error: parsing \"" << *i << "\""<< std::endl;
        }
        i++;
    }
}

void npdPerfDataParser::parsePerformanceValueItem( const std::string &inputValue, cPerformanceData &performanceItem )
{
    boost::smatch what;

    // std::cout << "analysing2: \"" << inputValue << "\"" << std::endl;
    if ( boost::regex_match( inputValue, what, this->performanceItemRegex, boost::match_perl ) ) {
        performanceItem.name = what[1];
        performanceItem.value = what[2];
        performanceItem.uom = what[3];
        this->parseWarningCriticalMinMax( what[4], performanceItem );
        this->parsePerformanceValueItemSuccessfullFlag = 1;
    } else {
        this->parsePerformanceValueItemSuccessfullFlag = 0;
    }
}

void npdPerfDataParser::parseWarningCriticalMinMax( const std::string &inputItem,
        cPerformanceData &performanceItem )
{
    boost::char_separator<char> sep(";");
    boost::tokenizer< boost::char_separator<char> > tokens(inputItem, sep);
    boost::tokenizer< boost::char_separator<char> >::iterator iter;
    int i;

    for ( i=0, iter = tokens.begin(); iter != tokens.end(); iter++, i++ ) {
        switch ( i ) {
        case 0:
            performanceItem.warning = *iter;
            break;
        case 1:
            performanceItem.critical = *iter;
            break;
        case 2:
            performanceItem.min = *iter;
            break;
        case 3:
            performanceItem.max = *iter;
            break;
        default:
            break;
        }
    }
}
/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

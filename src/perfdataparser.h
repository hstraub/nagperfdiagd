#ifndef __NAGPERFDIAG_PERFDATAPARSER__
#define __NAGPERFDIAG_PERFDATAPARSER__

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

#include "performancedata.h"
#include "npdconfig.h"

#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include <istream>
#include <string>
#include <vector>


typedef std::vector<cNpdData> vNpdData;
typedef boost::shared_ptr< vNpdData > spNpdDataVector;

class npdPerfDataParser
{
public:
    npdPerfDataParser( spcNpdConfiguration spNpdConfiguration );
    /*
    ~npdPerfDataParser( );
    */

    void parseDataStream( std::istream &inputDataStream, cNpdData &performanceData );
    spNpdDataVector getParsedData( void );

    void parseDataRecord( const std::string &dataRecord );
    void parseServicecheckcommand( const std::string &servicecheckcommand,
                                   std::string &parsedServicecheckcommand );
    void parseServicePerfDataRecord( const std::string &servicePerfData );
    void parsePerformanceValueItem( const std::string &inputValue,
                                    cPerformanceData &performanceItem );
    void parseWarningCriticalMinMax( const std::string &inputItem,
                                     cPerformanceData &performanceData );

private:
    spcNpdConfiguration spNpdConfiguration;
    spServicePerfData actualServicePerfDataRecord;
    int parsePerformanceValueItemSuccessfullFlag;

    boost::regex dataRecordRegex;
    boost::regex performanceDataRegex;
    boost::regex performanceItemRegex;
    boost::regex servicecheckcommandRegex;

};
#endif
/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

#ifndef __NAGPERFDIAG_NPDCONFIG__
#define __NAGPERFDIAG_NPDCONFIG__

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

#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include <istream>
#include <map>
#include <string>

typedef std::map<std::string, std::string> mcConfigurationParameter;
typedef std::map<std::string, std::string>::iterator mcConfigurationParameterIter;
typedef std::map<std::string, int> mcAllConfigurationParameter;
typedef std::map<std::string, int>::iterator mcAllConfigurationParameterIter;

class cNpdConfiguration
{
public:

    cNpdConfiguration( );

    void readConfiguration( std::istream &parameterStream );
    void parseConfigurationRecord( std::string &configurationRecord );
    void printToStream ( std::ostream &os );
    std::string getParameter( const char *pParameterName );

private:
    void initializeAllKnownParameterMap( void );
    void setupDefaultParameterValues( void );

private:

    mcConfigurationParameter mConfigurationParameter;
    mcAllConfigurationParameter mAllConfigurationParameter;
    boost::regex notRecordRegex;
    boost::regex dataRegex;
    mcAllConfigurationParameterIter notFound;
    mcConfigurationParameterIter parameterNotFound;
};

typedef boost::shared_ptr<cNpdConfiguration> spcNpdConfiguration;

#endif

/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

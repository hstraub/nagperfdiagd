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

#include "config.h"
#include "npdconfig.h"

#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>

#include <cassert>

static char varrundir[] = VARRUNDIR;
static char varlogdir[] = VARLOGDIR;

cNpdConfiguration::cNpdConfiguration( )
    : dataRegex( "^([^#=]+)=(.*)$" ),  notRecordRegex( "^ *#.*$" )
{
    notFound = mAllConfigurationParameter.end( );
    parameterNotFound = mConfigurationParameter.end( );
    mConfigurationParameter = mcConfigurationParameter( );
    initializeAllKnownParameterMap( );
}

void cNpdConfiguration::readConfiguration( std::istream &parameterStream )
{
    boost::smatch what;
    std::string configurationRecord;

    mConfigurationParameter = mcConfigurationParameter( );
    setupDefaultParameterValues( );


    while ( ! parameterStream.eof( ) ) {
        std::getline( parameterStream, configurationRecord );
        if ( boost::regex_match( configurationRecord, what, notRecordRegex ) ) {
            continue;
        } else {
            parseConfigurationRecord( configurationRecord );
        }
    }
}

std::string cNpdConfiguration::getParameter( const char *pParameterName )
{
    assert( mConfigurationParameter.find( pParameterName ) != parameterNotFound );

    return mConfigurationParameter[pParameterName];
}

void cNpdConfiguration::parseConfigurationRecord( std::string &configurationRecord )
{
    boost::smatch what;

    if ( boost::regex_match( configurationRecord, what, dataRegex ) ) {
        if ( mAllConfigurationParameter.find( what[1] ) != notFound ) {
            mConfigurationParameter[what[1]] = what[2];
        } else {
            std::cout << "Unbekannter Parameter: " << configurationRecord << std::endl;
        }
    }
    // FIXME: error handling
}

void cNpdConfiguration::printToStream ( std::ostream &os )
{
    mcConfigurationParameterIter iter;

    for ( iter = mConfigurationParameter.begin( ); iter != mConfigurationParameter.end( ); iter++ ) {
        os << "    " << iter->first << "=" << iter->second << std::endl;
    }
}

void cNpdConfiguration::initializeAllKnownParameterMap( void )
{
    mAllConfigurationParameter["nagios_input_data_path"] = 1;
    mAllConfigurationParameter["processing_interval"] = 1;
    mAllConfigurationParameter["nagios_input_data_filename_regex"] = 1;
    mAllConfigurationParameter["process_user"] = 1;
    mAllConfigurationParameter["process_group"] = 1;
    mAllConfigurationParameter["daemon_logfile"] = 1;
    mAllConfigurationParameter["pid_file"] = 1;
    mAllConfigurationParameter["daemon_logfile"] = 1;
    mAllConfigurationParameter["debug_output_directory"] = 1;
    mAllConfigurationParameter["db_type"] = 1;
    mAllConfigurationParameter["db_host"] = 1;
    mAllConfigurationParameter["db_databasename"] = 1;
    mAllConfigurationParameter["db_username"] = 1;
    mAllConfigurationParameter["db_password"] = 1;
}

void cNpdConfiguration::setupDefaultParameterValues( void )
{
    mConfigurationParameter["nagios_input_data_path"] = "/tmp";
    mConfigurationParameter["processing_interval"] = "30";
    mConfigurationParameter["nagios_input_data_filename_regex"]
        = "service-perfdata\\.[\\d+]*\\.nagperfdiag";
    mConfigurationParameter["process_user"] = "nagios";
    mConfigurationParameter["process_group"] = "nagios";
    mConfigurationParameter["daemon_logfile"] = varlogdir;
    mConfigurationParameter["daemon_logfile"] += "/nagperfdiagd.log";
    mConfigurationParameter["pid_file"] = varrundir;
    mConfigurationParameter["pid_file"] += "/nagperfdiagd.pid";
    mConfigurationParameter["debug_output_directory"] = varlogdir;
    mConfigurationParameter["debug_output_directory"] += "/debug";
    mConfigurationParameter["rrd_default_datatype"] = "GAUGE";
    mConfigurationParameter["db_type"] = "postgresql";
    mConfigurationParameter["db_host"] = "localhost";
    mConfigurationParameter["db_databasename"] = "nagperfdiag";
    mConfigurationParameter["db_username"] = "dbuser";
    mConfigurationParameter["db_password"] = "dbpassword";
}

/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

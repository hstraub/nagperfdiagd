#ifndef __NAGPERFDIAG_PERFORMANCEDATA__
#define __NAGPERFDIAG_PERFORMANCEDATA__

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
#include <string>
#include <vector>

class cPerformanceData
{

public:
    void printToStream( std::ostream &os ) {
        os << "Name: " << this->name
        << " value:" << this->value
        << " uom: " << this->uom
        << " warning: " << this->warning
        << " critical: " << this->critical
        << " min: "	<< this->min
        << " max: "  << this->max
        << " rrdDataType: " << this->rrdDataType
        << " rrdFilename: " << this->rrdFilename
        << " rrdDS: " << this->rrdDS
        << std::endl;
    };

    std::string name;
    std::string value;
    std::string uom;
    std::string warning;
    std::string critical;
    std::string min;
    std::string max;
    std::string rrdDataType;
    std::string rrdFilename;
    std::string rrdDS;
};
typedef std::map<std::string, cPerformanceData> mcValueToPerformanceData;
typedef std::map<std::string, cPerformanceData>::iterator mcValueToPerformanceDataIter;

// from rrdmanager.h
typedef std::map<std::string, std::string> mcRrdFilenameToPerformanceValueName;
typedef std::map<std::string, std::string>::iterator mcRrdFilenameToPerformanceValueNameIter;
class cRrdNameComponents
{
public:
    void printToStream( std::ostream &os ) {
        os << "  rrdNameComponents:" << std::endl;
        os << "    serviceDirectory: " << serviceDirectory << std::endl;
        os << "    mRrdFilenameToPerformanceValueName.size:"
        << mRrdFilenameToPerformanceValueName.size( ) << std::endl;
        for ( mcRrdFilenameToPerformanceValueNameIter iter =
                    mRrdFilenameToPerformanceValueName.begin( );
                iter != mRrdFilenameToPerformanceValueName.end( );
                iter++ ) {
            os << "    value: " << iter->second << " " << iter->first << std::endl;
        }
    };

public:
    std::string baseDirectory;
    std::string hostDirectory;
    std::string serviceDirectory;
    mcRrdFilenameToPerformanceValueName mRrdFilenameToPerformanceValueName;
};

class cRrdOperationLog
{
public:
    enum eRrdStatus { undefined, success, error };
    time_t timestamp;
    eRrdStatus rrdStatus;
    std::string rrdFilename;
    std::string valuename;
    std::string rrdOperation;
    std::string rrdOperationString;
    std::string rrdStatusText;
};

class cRrdOperationsLogger
{
public:

    void logSuccess( const std::string &rrdFilename, const std::string &valuename, 
        const std::string &rrdOperation,
        const std::string &rrdOperationString ) {
        cRrdOperationLog logEntry;
        logEntry.timestamp = 0;
        logEntry.rrdFilename = rrdFilename;
        logEntry.valuename = valuename;
        logEntry.rrdOperation = rrdOperation;
        logEntry.rrdOperationString = rrdOperationString;
        vRrdOperationsLog.push_back( logEntry );
    };

    void logFailure( const std::string &rrdFilename, const std::string &valuename,
        const std::string &rrdOperation,
        const std::string &rrdOperationString, const std::string &rrdStatusText ) {
        cRrdOperationLog logEntry;
        logEntry.timestamp = 0;
        logEntry.rrdFilename = rrdFilename;
        logEntry.valuename = valuename;
        logEntry.rrdOperation = rrdOperation;
        logEntry.rrdOperationString = rrdOperationString;
        logEntry.rrdStatusText = rrdStatusText;
        iMaxFailureStatus = 1;
        vRrdOperationsLog.push_back( logEntry );
    };

    void printToStream( std::ostream &os ) {
        os << "  RrdOperationsLogger maxFailureStatus " << iMaxFailureStatus << std::endl;
        for ( std::vector<cRrdOperationLog>::iterator iter = vRrdOperationsLog.begin( );
                iter != vRrdOperationsLog.end( );
                iter++) {
            os << "   timestamp: " << iter->timestamp
                << " Filename: " << iter->rrdFilename << std::endl;
            os << "     Value: " << iter->valuename;
            os << "     OperationStatus: ";
            if ( iter->rrdStatus == cRrdOperationLog::success ) {
                os << "success";
            } else if ( iter->rrdStatus == cRrdOperationLog::error ) {
                os << "error";
            }
            os << std::endl;
            os << "     operation: " << iter->rrdOperation << std::endl;;
            os << "     rrdOperationString: " << iter->rrdOperationString << std::endl;
            os << "     ErrorText: " << iter->rrdStatusText << std::endl;
        }
    };

    int isAnyOperationWithFailure( void );

public:
    std::vector<cRrdOperationLog> vRrdOperationsLog;

private:
    int iMaxFailureStatus;
};


class cServicePerfData
{
public:
    void printToStream( std::ostream &os ) {
        os << "Performance Data for hostname: " << this->hostname << std::endl;
        os << "    servicedesc: " << this->servicedesc << std::endl;
        os << "    servicecheckcommand: " << this->servicecheckcommand << std::endl;

        os << "  Nagios Data:" << std::endl;
        std::map<std::string, std::string>::iterator nagiosIter;
        for (
            nagiosIter = mNagiosData.begin( );
            nagiosIter != mNagiosData.end( );
            nagiosIter++) {
            os << "    " << nagiosIter->first << " :: "
            << nagiosIter->second << std::endl;
        }

        mcValueToPerformanceDataIter iter;
        os << "  Itemlist: " << std::endl;
        for ( iter = mValueToPerformanceData.begin( );
                iter != mValueToPerformanceData.end( ); iter ++ ) {
            os << "    ";
            iter->second.printToStream( os );
        }

        rrdNameComponents.printToStream( os );
        rrdOperationsLogger.printToStream( os );
    };

    int getNumberOfPerformanceValues( void ) {
        return mValueToPerformanceData.size( );
    };

    std::string hostname;

    std::string servicedesc;
    std::string servicecheckcommand;
    mcValueToPerformanceData mValueToPerformanceData;
    std::map<std::string, std::string> mNagiosData;
    cRrdNameComponents rrdNameComponents;
    cRrdOperationsLogger rrdOperationsLogger;
};
typedef boost::shared_ptr<cServicePerfData> spServicePerfData;
typedef std::vector<spServicePerfData> vspServicePerfData;
typedef std::vector<spServicePerfData>::iterator vspServicePerfDataIter;

class cNpdData
{
public:
    /*
    cNpdData( );
    ~cNpdData( );
    */
    void printToStream( std::ostream &os ) {
        std::vector<spServicePerfData>::iterator iter;
        os << "Performance Block: " << std::endl;
        for ( iter = performanceVector.begin( ); iter != performanceVector.end( ); iter++ ) {
            (*iter)->printToStream( os );
        }
    };

    void setData( std::string &name, std::string &value );
    std::string getData( std::string &name );

    vspServicePerfData performanceVector;
    vspServicePerfData performanceVectorWithoutPerformanceValues;
};

typedef boost::shared_ptr<cNpdData> spcNpdData;
typedef std::vector<spcNpdData> vspcNpdData;

#endif
/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

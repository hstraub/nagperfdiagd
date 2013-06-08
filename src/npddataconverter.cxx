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

#include "npddataconverter.h"

#include <boost/lexical_cast.hpp>
#include <iostream>

cNpdDataAndTypeConverter::cNpdDataAndTypeConverter( spcNpdConfiguration spNpdConfiguration,
    spcNpdTypeCache spNpdTypeCache )
    : spNpdConfiguration( spNpdConfiguration ), spNpdTypeCache( spNpdTypeCache )
{
    
}

void cNpdDataAndTypeConverter::processingInputData( spcNpdData spNpdData )
{
    vspServicePerfDataIter perfDataIter;

    for ( perfDataIter = spNpdData->performanceVector.begin( );
            perfDataIter != spNpdData->performanceVector.end( );
            perfDataIter++ ) {
        convertingUOMData( *perfDataIter );
        determineRrdDataType ( *perfDataIter );
    }
}

void cNpdDataAndTypeConverter::convertingUOMData( spServicePerfData pServicePerfData )
{
    mcValueToPerformanceDataIter valueToPerformanceIter;
    std::string sUom, sValue, sWarning, sCritical, sMin, sMax;
    std::string dUom, dValue, dWarning, dCritical, dMin, dMax;
    float fFactor;
    int convertFlag;

    for ( valueToPerformanceIter = pServicePerfData->mValueToPerformanceData.begin( );
            valueToPerformanceIter !=  pServicePerfData->mValueToPerformanceData.end( );
            valueToPerformanceIter++ ) {

        sUom = valueToPerformanceIter->second.uom;
        sValue = valueToPerformanceIter->second.value;
        sWarning = valueToPerformanceIter->second.warning;
        sCritical = valueToPerformanceIter->second.critical;
        sMin = valueToPerformanceIter->second.min;
        sMax = valueToPerformanceIter->second.max;

        convertFlag = 1;
        if ( sUom == "ms" ) {
            fFactor = 1E-3;
            dUom = "s";
        } else if ( sUom == "us" ) {
            fFactor = 1E-6;
            dUom = "s";
        } else if ( sUom == "ns" ) {
            fFactor = 1E-9;
            dUom = "s";
        } else {
            convertFlag = 0;
        }

        if ( convertFlag == 1 ) {
            calculateNewValue( fFactor, sValue, sWarning, sCritical, sMin, sMax,
                dValue, dWarning, dCritical, dMin, dMax );

            std::cout << "UOM Convert for host " << pServicePerfData->hostname
                << " and service: " << pServicePerfData->servicedesc
                << " and valuename: " << valueToPerformanceIter->second.name
                << " converting " << valueToPerformanceIter->second.value << sUom
                << " to " << dValue << dUom << std::endl;

            valueToPerformanceIter->second.value = dValue;
            valueToPerformanceIter->second.warning = dWarning;
            valueToPerformanceIter->second.critical= dCritical;
            valueToPerformanceIter->second.min = dMin;
            valueToPerformanceIter->second.max = dMax;
            valueToPerformanceIter->second.uom = dUom;
        }
    }
}

void cNpdDataAndTypeConverter::calculateNewValue( float fFactor, const std::string &sValue,
    const std::string &sWarning, const std::string &sCritical,
    const std::string &sMin, const std::string &sMax, 
    std::string &dValue, std::string &dWarning, std::string &dCritical,
    std::string &dMin, std::string &dMax )
{
    float fValue, fMin, fMax, fWarning, fCritical;

        fValue = boost::lexical_cast<float>( sValue );
        fValue = fValue * fFactor;
        dValue = boost::lexical_cast<std::string>( fValue );

        if ( sWarning != "" ) {
            fWarning = boost::lexical_cast<float>( sWarning );
            fWarning = fWarning * fFactor;
            dWarning = boost::lexical_cast<std::string>( fWarning );
        }

        if ( sCritical != "" ) {
            fCritical = boost::lexical_cast<float>( sCritical );
            fCritical = fCritical * fFactor;
            dCritical = boost::lexical_cast<std::string>( fCritical );
        }

        if ( sMin != "" ) {
            fMin = boost::lexical_cast<float>( sMin );
            fMin = fMin * fFactor;
            dMin = boost::lexical_cast<std::string>( fMin );
        }

        if ( sMax != "" ) {
            fMax = boost::lexical_cast<float>( sMax );
            fMax = fMax * fFactor;
            dMax = boost::lexical_cast<std::string>( fMax );
        }
}

void cNpdDataAndTypeConverter::determineRrdDataType( spServicePerfData pServicePerfData )
{
    mcValueToPerformanceDataIter valueToPerformanceIter;

    for ( valueToPerformanceIter = pServicePerfData->mValueToPerformanceData.begin( );
            valueToPerformanceIter !=  pServicePerfData->mValueToPerformanceData.end( );
            valueToPerformanceIter++ ) {
            spNpdTypeCache->getDataTypeFor( pServicePerfData->hostname,
            pServicePerfData->servicedesc,
            pServicePerfData->servicecheckcommand,
            valueToPerformanceIter->second.name, 
            valueToPerformanceIter->second.rrdDataType );
    }
}

/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

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

#include "npdrrdnameconverter.h"

#include <boost/lexical_cast.hpp>
#include <iostream>

cNpdDataAndTypeConverter::cNpdDataAndTypeConverter( spcNpdConfiguration spNpdConfiguration,
    spcNpdTypeCache spNpdTypeCache )
    : spNpdConfiguration( spNpdConfiguration ), spNpdTypeCache( spNpdTypeCache ), 
    rrdDirectory( "/tmp/testrrd" )
{
    
}

void cNpdRrdNameConverter::processingInputData( spcNpdData spNpdData )
{
    vspServicePerfDataIter perfDataIter;

    for ( perfDataIter = spNpdData->performanceVector.begin( );
            perfDataIter != spNpdData->performanceVector.end( );
            perfDataIter++ ) {
        createRrdNameComponentsFromPerformanceData( *perfDataIter );
    }
}

void cNpdRrdNameConverter::createRrdNameComponentsFromPerformanceData( spServicePerfData pServiceItem )
{
    for ( valueIter = pServiceItem->mValueToPerformanceData.begin( );
            valueIter != pServiceItem->mValueToPerformanceData.end( );
            valueIter++) {
        // TODO: now convert to valid Directory Names and Filename and DS
        pServiceItem->rrdFilename = this->rrdDirectory + "/" + pServiceItem->hostname + "/"
            + pServiceItem->servicedesc + "/" + valueIter->first + ".rrd";
        pServiceItem->rrdDS = valueIter->first;
    }
}

/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

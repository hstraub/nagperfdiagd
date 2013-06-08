#ifndef __NAGPERFDIAG_NPDDATACONVERTER__
#define __NAGPERFDIAG_NPDDATACONVERTER__

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
#include "npdtypecache.h"

class cNpdDataAndTypeConverter
{
public:
    cNpdDataAndTypeConverter( spcNpdConfiguration spNpdConfiguration,
        spcNpdTypeCache spNpdTypeCache );

    void processingInputData( spcNpdData spNpdData );

private:
    spcNpdConfiguration spNpdConfiguration;
    spcNpdTypeCache spNpdTypeCache;
    void convertingUOMData( spServicePerfData pServicePerfData );
    void determineRrdDataType( spServicePerfData pServicePerfData );
    void calculateNewValue( float fFactor,
    const std::string &sValue, const std::string &sWarning, const std::string &sCritical,
    const std::string &sMin, const std::string &sMax,
    std::string &dValue, std::string &dWarning, std::string &dCritical,
    std::string &dMin, std::string &dMax );
};

#endif

/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

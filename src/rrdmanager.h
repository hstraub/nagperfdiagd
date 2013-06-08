#ifndef __NAGPERFDIAG_RRDMANAGER__
#define __NAGPERFDIAG_RRDMANAGER__

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
#include "rrdmanager.h"
#include "perfdataparser.h"
#include <sys/stat.h>

class cRrdManager
{
public:
    cRrdManager( );

    void checkRrdsAndCreateIfNotAvailable( cNpdData &inputData );
    void updateRrdsWithPerformanceData( cNpdData &inputData );

private:
    int isDirectoryAvailable( const std::string directoryName );
    void createDirectory( const std::string directoryName );
    int isRrdUpdatePossible( const std::string rrdFilename );
    void createSingleRrd( const std::string rrdName, spServicePerfData servicePerfData,
        std::string valueName );
    void fillRraDataInVector( std::vector<std::string> &vRraData );
    void buildRrdNameComponentsFromPerformanceData( spServicePerfData pServiceItem );

private:
    int rrd_create( std::string rrdName, std::string step, std::string start,
        std::vector<std::string> vDsAndRra);
    int rrd_update( std::string rrdName, std::string rrdTemplate,
        std::vector<std::string> vRrdUpdateStrings );

private:
    typedef std::map<std::string, int> mcRrdFilenameCache;

    std::string rrdDirectory;
    mode_t dirMode;
    mode_t rrdMode;
    std::string rrdErrorText;
    std::map<std::string, std::vector<spServicePerfData> > mUpdatesSortedByRrdname;
    mcRrdFilenameCache mRrdFilenameCache;
};

#endif
/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

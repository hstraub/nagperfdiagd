#ifndef __NAGPERFDIAG_NPDLOGGER__
#define __NAGPERFDIAG_NPDLOGGER__

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

#include <signal.h>
#include <soci/soci.h>
#include <soci/soci-postgresql.h>
#include <unistd.h>

class cNpdLogger
{
public:

    cNpdLogger( spcNpdConfiguration spNpdConfiguration );
    int isDebugDirectoryAvailableAndWriteable( void );  
    void writeRrdLogsToDatabase( spcNpdData spNpdData );

private:
    void connectToSql( void );
    void insertOrUpdateLogs( const std::string & hostname, const std::string &servicedesc,
        const std::string &valuename, const std::string &timet, const std::string &datatype,
        const std::string &checkcommand );

private:
    spcNpdConfiguration spNpdConfiguration;
    int outputDebugDirectoryIsWriteable;
    soci::session sqlSession;
};

#endif

/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

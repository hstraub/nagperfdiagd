#ifndef __NAGPERFDIAG_NPDPROCESS__
#define __NAGPERFDIAG_NPDPROCESS__

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
#include <unistd.h>

class cNpdProcess
{
public:

    cNpdProcess( );

    int readCmdlOptions( int argc, char *argv[] );
    int readConfiguration( void );
    int prepareDaemonMode( void );
    int runMainloop( void );

public:
    enum status { success, failure, failure_continue };

private:
    spcNpdData readNagiosPerformanceDataFiles( void );
    void removeNagiosInputFiles( std::vector<std::string> &vFileList );
    int switchToUidGid( void );
    int forkAndDetach( void );
    void openLogsAndSetPermissions( void );
    void installSignalHandlers( void );
    void pidFileCreate( void );
    void pidFileRemove( void );

private:
    void buildNagiosInputDataFileList( std::vector<std::string> &vFileList );
    boost::regex sNagiosInputDataFilenameRegex;
    spcNpdConfiguration spNpdConfiguration;
    unsigned int sleepTime;
    pid_t pid;

};

#endif

/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

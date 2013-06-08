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
#include "npdlogger.h"
#include "npdprocess.h"
#include "perfdataparser.h"
#include "rrdmanager.h"

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <fstream>
#include <string>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>
#include <syslog.h>

// Signal Handling
static int global_stopMainLoop;
static int global_sighup;
extern "C" {
static void sig_term_handler( int signal );
};


cNpdProcess::cNpdProcess( )
    : pid( 0 )
{
}

int cNpdProcess::readCmdlOptions( int argc, char *argv[] )
{
}

int cNpdProcess::readConfiguration( void)
{
    spNpdConfiguration = spcNpdConfiguration( new cNpdConfiguration( ) );
    std::ifstream configurationStream( "nagperfdiagd.conf" );
    spNpdConfiguration->readConfiguration( configurationStream );
    configurationStream.close( );

    std::cout << "die ganze Parameter Map" << std::endl;
    spNpdConfiguration->printToStream( std::cout );

    // FIXME: better place?
    sNagiosInputDataFilenameRegex = 
        spNpdConfiguration->getParameter( "nagios_input_data_filename_regex" );

    sleepTime = 
        boost::lexical_cast<unsigned int>( spNpdConfiguration->getParameter ("processing_interval" ) );
}
int cNpdProcess::prepareDaemonMode( void )
{
    if ( switchToUidGid( ) == failure ) {
        std::cout << "Error: switchToUidGid returns an error" << std::endl;
        return failure;
    }

    if ( forkAndDetach( ) == failure ) {
        std::cout << "Error: forkAndDetach returns an error" << std::endl;
        return failure;
    }

    installSignalHandlers( );
    
    return success;
}

int cNpdProcess::runMainloop( void)
{
    npdPerfDataParser dataParser ( spNpdConfiguration );
    spcNpdTypeCache spNpdTypeCache = spcNpdTypeCache( new cNpdTypeCache( spNpdConfiguration ) ) ;
    cNpdDataAndTypeConverter npdDataAndTypeConverter( spNpdConfiguration, spNpdTypeCache );
    cNpdLogger npdLogger( spNpdConfiguration );

    global_stopMainLoop = 0;
    global_sighup = 0;

    while( ! global_stopMainLoop ) {
        spcNpdData spPerformanceData = spcNpdData( new cNpdData );
        std::vector<std::string> vFileList;

        buildNagiosInputDataFileList( vFileList );
        for ( std::vector<std::string>::iterator iter = vFileList.begin( );
                iter != vFileList.end();
                iter++ ) {
            std::ifstream inputDataStream( iter->c_str( ) );
            dataParser.parseDataStream( inputDataStream, *spPerformanceData );
            inputDataStream.close( );
        }

        std::cout << "Ergebnisrecords mit Performancwerten: "
            << spPerformanceData->performanceVector.size( ) << std::endl;
        std::cout << "Ergebnisrecords ohne Performancewerter:"
            << spPerformanceData->performanceVectorWithoutPerformanceValues.size( )
            << std::endl;

        npdDataAndTypeConverter.processingInputData ( spPerformanceData );

        cRrdManager rrdManager;
        rrdManager.checkRrdsAndCreateIfNotAvailable( *spPerformanceData );
        rrdManager.updateRrdsWithPerformanceData( *spPerformanceData );
        
        removeNagiosInputFiles( vFileList );
        npdLogger.writeRrdLogsToDatabase( spPerformanceData );
        spPerformanceData->printToStream( std::cout );

        sleep( 10 );
        if ( global_sighup ) {
            global_sighup = 0;
            std::cout << "Reloading Configuration (SIGHUP)" << std::endl;
        }
    }

    pidFileRemove( );
    std::cout << "Leaving MainLoop; terminating" << std::endl;
}

void cNpdProcess::removeNagiosInputFiles( std::vector<std::string> &vFileList )
{
std::vector<std::string>::iterator iter;

for ( iter = vFileList.begin( ); iter != vFileList.end( ); iter++ ) {
    std::cout << "Deleting " << *iter << std::endl;
    remove( iter->c_str( ) );
}
}

spcNpdData cNpdProcess::readNagiosPerformanceDataFiles( void )
{

}

void cNpdProcess::buildNagiosInputDataFileList( std::vector<std::string> &vFileList )
{
    int status;
    DIR *fd;
    struct dirent *pDirentBuffer;
    boost::smatch what;
    std::string sInputDataPath = spNpdConfiguration->getParameter( "nagios_input_data_path" );

    fd = opendir( sInputDataPath.c_str( ) );
    if ( fd == NULL ) {
        std::cout << "Error bei diropen" << std::endl;
        return;
    }

    while ( ( pDirentBuffer = readdir( fd) ) != NULL ) {
        std::string sFilename = std::string( pDirentBuffer->d_name );
        if ( boost::regex_match( sFilename, what, sNagiosInputDataFilenameRegex ) ) {
            std::cout << "Dir Entry: " << sFilename << std::endl;
            vFileList.push_back( sInputDataPath + "/" + sFilename );
        }
    }

    closedir( fd );
}

int cNpdProcess::switchToUidGid( void )
{
    struct passwd *pUser;
    struct group *pGroup;
    uid_t currentUid;
    gid_t currentGid;

    currentUid = getuid( );
    currentGid = getgid( );

    pGroup = getgrnam( spNpdConfiguration->getParameter( "process_group" ).c_str( ) );
    if ( pGroup == NULL ) {
        std::cout << "Error getgrnam" << std::endl;
        return failure;
    }
    pUser = getpwnam( spNpdConfiguration->getParameter( "process_user" ).c_str( ) );
    if ( pGroup == NULL ) {
        std::cout << "Error getpwnam" << std::endl;
        return failure;
    }

    if ( currentUid == pUser->pw_uid && currentGid == pGroup->gr_gid ) {
        return success;
    }

    if ( currentGid != pGroup->gr_gid ) {
        if ( setregid( pGroup->gr_gid, pGroup->gr_gid ) != 0 ) {
            std::cout << "Error setregid" << strerror( errno ) << std::endl;
            return failure;
        }
    }

    if ( currentUid != pUser->pw_uid ) {
        if ( setreuid( pUser->pw_uid, pUser->pw_uid ) != 0 ) {
            std::cout << "Error setreuid" << strerror( errno ) << std::endl;
            return failure;
        }
    }

    return success;
}

int cNpdProcess::forkAndDetach( void )
{

    switch( ( pid = fork( ) ) ) {
    case -1:
        std::cout << "Error: cannot fork" << std::endl;
        exit( 1 );
    case 0:
        if ( setsid( ) == -1 ) {
            std::cout << "Error: cannot setsid" << std::endl;
        }

        pid = getpid( );
        pidFileCreate( );

        close( STDIN_FILENO );
        close( STDOUT_FILENO );
        close( STDERR_FILENO );

        openLogsAndSetPermissions( );

        break;
    default:
        exit( 0 );
}

return success;
}

void cNpdProcess::openLogsAndSetPermissions( void )
{
    mode_t openMode;
    struct stat fileStatBuffer;
    int status;
    std::string logfile = spNpdConfiguration->getParameter( "daemon_logfile" );

    if ( stat( logfile.c_str( ), &fileStatBuffer ) == 0 ) {
        openMode = O_WRONLY | O_APPEND;
    } else {
        openMode = O_WRONLY | O_CREAT;
    }

    if ( open( "/dev/null", O_RDWR ) < 0 ) {
        std::cout << "Error opening /dev/null" << std::endl;
        return; //FIXME: exception
    }

     if ( open( logfile.c_str( ) , O_WRONLY | O_APPEND| O_CREAT) < 0 ) {
        std::cout << "Error opening log file: " << logfile << " error: "
            << strerror( errno ) << std::endl;
        return; //FIXME: exception
    }

    if ( chmod( logfile.c_str( ), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ) != 0 ) {
        std::cout << "Error chmod on: " << logfile << " error: "
            << strerror( errno ) << std::endl;
        return; //FIXME exception
    }

    if ( dup( 1 ) != 2 ) {
        std::cout << "Error dup(1); error: " << strerror( errno ) << std::endl;
        return; //FIXME: exception
    }
}

void cNpdProcess::installSignalHandlers( void )
{
    struct sigaction sigAction;
    sigAction.sa_handler = &sig_term_handler;
    sigemptyset( &sigAction.sa_mask );
    sigAction.sa_flags = 0;
    if ( sigaction( SIGTERM, &sigAction, NULL ) != 0 ) {
        std::cerr << "Error: cannot install the SIGTERM handler" << std::endl;
        exit ( 1 ); //FIXME exception
    }
    if ( sigaction( SIGHUP, &sigAction, NULL ) != 0 ) {
        std::cerr << "Error: cannot install the SIGHUP handler" << std::endl;
        exit ( 1 ); //FIXME exception
    }
}

void cNpdProcess::pidFileCreate( void )
{
    std::string sPidFilename = spNpdConfiguration->getParameter( "pid_file" );
    std::fstream pidStream;
    struct stat statBuffer;

    assert( pid != 0 );

    if ( stat( sPidFilename.c_str( ), &statBuffer ) == 0 ) {
        std::cout << "Error another pid file is here: " << sPidFilename << std::endl;
        exit ( 1 ); //FIXME exception
    }
    pidStream.open( sPidFilename.c_str( ), std::ios_base::out);
    if ( pidStream.fail( ) ) {
        std::cout << "Error writing " << sPidFilename << std::endl;
        // FIXME: exception
    }
    pidStream << pid << std::endl;
    pidStream.close( );
}

void cNpdProcess::pidFileRemove( void )
{
    std::string sPidFilename = spNpdConfiguration->getParameter( "pid_file" );

    assert( pid != 0 );
    if ( unlink( sPidFilename.c_str( ) ) != 0 ) {
        std::cout << "Error unlink " << sPidFilename << "; errortext: "
            << strerror( errno ) << std::endl;
    }
}

// Plain C Signal Hander Functions
void sig_term_handler( int signal )
{
    //reinstalling the Signal Handler
    struct sigaction sigAction;
    sigAction.sa_handler = &sig_term_handler;
    sigemptyset( &sigAction.sa_mask );
    sigAction.sa_flags = 0;
    switch( signal ) {
    case SIGTERM:
        global_stopMainLoop = 1;
        //sigaction( SIGTERM, &sigAction, NULL );
        break;
    case SIGHUP:
        global_sighup = 1;
        sigaction( SIGHUP, &sigAction, NULL );
        break;
    default:
        break;
    }
}

/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

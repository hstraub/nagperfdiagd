#ifndef __NAGPERFDIAG_NPDTYPECACHE__
#define __NAGPERFDIAG_NPDTYPECACHE__

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

#include <boost/shared_ptr.hpp>
#include <soci/soci.h>
#include <soci/soci-postgresql.h>


class cNpdTypeCache
{
public:
    cNpdTypeCache( spcNpdConfiguration spNpdConfiguration );
    void getDataTypeFor( const std::string &hostname, const std::string &servicedesc, 
        const std::string &checkcommand, const std::string &valueName, std::string &dataType );

private:
    void connectToSql( void );
    void readFromSql ( const std::string &checkcommand,
        const std::string &valueName, std::string &dataType );


private:
    spcNpdConfiguration spNpdConfiguration;
    soci::session sqlSession;
    std::map<std::string,std::string> mCheckcommandsToDataType;
    std::map<std::string,std::string>::iterator mCheckcommandsToDataTypeIterNotFound;
};

typedef boost::shared_ptr<cNpdTypeCache> spcNpdTypeCache;

#endif

/* vim: set ai tabstop=4 shiftwidth=4 expandtab: */

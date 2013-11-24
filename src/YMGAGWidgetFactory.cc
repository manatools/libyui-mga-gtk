/*
  Copyright 2013 by Angelo Naselli <anaselli at linux dot it>
 
  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) version 3.0 of the License. This library
  is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details. You should have received a copy of the GNU
  Lesser General Public License along with this library; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
  Floor, Boston, MA 02110-1301 USA
*/


/*-/

  File:         YMGAGWidgetFactory.cc

  Author:       Angelo Naselli <anaselli@linux.it>

/-*/

#define YUILogComponent "mga-gtk"
#include <yui/YUILog.h>

#include "YMGAGWidgetFactory.h"
#include <yui/gtk/YGUI.h>
#include <yui/YUIException.h>
#include <YExternalWidgets.h>


#include <string>

#include "YMGA_GCBTable.h"

using std::string;


YMGAGWidgetFactory::YMGAGWidgetFactory()
    : YMGAWidgetFactory()
{
    // NOP
}


YMGAGWidgetFactory::~YMGAGWidgetFactory()
{
    // NOP
}


YMGA_CBTable *
YMGAGWidgetFactory::createCBTable( YWidget * parent, YTableHeader * header, YTableMode mode )
{
    YMGA_GCBTable * table = new YMGA_GCBTable( parent, header, mode );
    YUI_CHECK_NEW( table );

    return table;
}


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

  File:		YMGAGWidgetFactory.h

  Author:       Angelo Naselli <anaselli@linux.it>

/-*/

#ifndef YMGAGWidgetFactory_h
#define YMGAGWidgetFactory_h


#include <yui/mga/YMGAWidgetExtensionFactory.h>

#include "YMGA_CBTable.h"


using std::string;


/**
 * Concrete widget factory for mandatory widgets.
 **/
class YMGAGWidgetFactory: public YMGAWidgetFactory
{
public:

  virtual YMGA_CBTable * createCBTable ( YWidget * parent, YTableHeader * header_disown, YCBTableMode mode = YCBTableCheckBoxOnFirstColumn );
  virtual YMGAMenuBar  * createMenuBar ( YWidget *parent );

protected:

    friend class YGWE;

    /**
     * Constructor.
     *
     * Use YUI::widgetFactory() to get the singleton for this class.
     **/
    YMGAGWidgetFactory();

    /**
     * Destructor.
     **/
    virtual ~YMGAGWidgetFactory();

}; // class YWidgetFactory


#endif // YMGAGWidgetFactory_h

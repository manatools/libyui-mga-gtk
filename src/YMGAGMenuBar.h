/*
  Copyright 2020 by Angelo Naselli <anaselli at linux dot it>

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

  File:         YMGAGMenuBar.h

  Author:       Angelo Naselli <anaselli@linux.it>

/-*/

#ifndef YMGAGMenuBar_h
#define YMGAGMenuBar_h


#include <yui/mga/YMGAMenuBar.h>
#include <yui/gtk/ygtktreeview.h>

#include <yui/gtk/YGSelectionStore.h>
#include <yui/gtk/YGWidget.h>

#include <gtk/gtk.h>



class YMGAGMenuBar : public YMGAMenuBar, public YGWidget
{
public:

  YMGAGMenuBar ( YWidget *parent );

  virtual ~YMGAGMenuBar ( );

  /**
  * Add an YMenuItem first item represents the menu name, other sub items menu entries
  *
  * Reimplemented from YSelectionWidget.
  **/
  virtual void addItem( YItem * item );

  /**
  * Enable YMGAMenuItem (menu name or menu entry) to enable/disable it into menubar or menu
  *
  * Reimplemented from YMGAMenuBar.
  **/
  virtual void enableItem(YItem * menu_item, bool enable=true);

  YGWIDGET_IMPL_COMMON (YMGAMenuBar)


private:
  struct Private;
  Private *d;

  void doCreateMenu (GtkWidget *menu, YItemIterator begin, YItemIterator end);
};

#endif // YMGAGMenuBar_h

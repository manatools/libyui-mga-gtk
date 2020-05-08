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

  File:         YMGAGMenuBar.cc

  Author:       Angelo Naselli <anaselli@linux.it>

/-*/

#include <yui/gtk/YGi18n.h>

#define YUILogComponent "mga-gtk-ui"

#include <yui/YUILog.h>
#include <yui/gtk/YGUI.h>
#include <yui/gtk/YGUtils.h>
#include <yui/gtk/YGWidget.h>
#include <YSelectionWidget.h>
#include <yui/gtk/YGSelectionStore.h>
#include <yui/gtk/ygtktreeview.h>
#include <string.h>
#include <yui/mga/YMGA_CBTable.h>
#include <YTable.h>

#include <yui/gtk/YGDialog.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmenubar.h>
#include <gtk/gtk.h>

#include "YMGAGMenuBar.h"


#define YGTK_VBOX_NEW(arg) gtk_box_new(GTK_ORIENTATION_VERTICAL,arg)
#define YGTK_HBOX_NEW(arg) gtk_box_new(GTK_ORIENTATION_HORIZONTAL,arg)

struct YMGAGMenuBar::Private
{
  GtkWidget *menubar;
};

YMGAGMenuBar::YMGAGMenuBar(YWidget* parent)
  :  YMGAMenuBar(NULL),
  YGWidget(this, parent, gtk_menu_bar_get_type(), NULL),
  d(new Private)

{
  YUI_CHECK_NEW ( d );
  d->menubar = getWidget();

  //gtk_misc_set_alignment (GTK_MISC (getWidget()), 0.0, 0.5);
}

static void selected_menuitem(GtkMenuItem *menuitem, YItem *item)
{
  YGUI::ui()->sendEvent (new YMenuEvent (item));
}

void YMGAGMenuBar::doCreateMenu (GtkWidget *menu, YItemIterator begin, YItemIterator end)
{
  for (YItemIterator it = begin; it != end; it++)
  {
    GtkWidget *entry;
    std::string str = YGUtils::mapKBAccel ((*it)->label());
    entry = gtk_menu_item_new_with_mnemonic (str.c_str());

    if ((*it)->hasChildren()) {
      GtkWidget *submenu = gtk_menu_new();
      //gtk_menu_item_set_submenu(GTK_MENU_ITEM(submenu), menu);
      gtk_menu_item_set_submenu(GTK_MENU_ITEM(entry), submenu);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
      gtk_widget_show(entry);

      doCreateMenu (submenu, (*it)->childrenBegin(), (*it)->childrenEnd());

    }
    else
    {
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
      gtk_widget_show(entry);
      g_signal_connect (G_OBJECT (entry), "activate",
                        G_CALLBACK (selected_menuitem), *it);
    }
  }
}


void YMGAGMenuBar::addItem(YItem* yitem)
{
  YMenuItem * item = dynamic_cast<YMenuItem *> ( yitem );
  YUI_CHECK_PTR ( item );

  // TODO icon from item
  GtkWidget *menu = gtk_menu_new();

  std::string lbl = YGUtils::mapKBAccel (item->label());
  GtkWidget *menu_entry = gtk_menu_item_new_with_mnemonic(lbl.c_str());
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_entry), menu);
  gtk_widget_show(menu_entry);
  gtk_menu_shell_append (GTK_MENU_SHELL (d->menubar), menu_entry);

  if (item->hasChildren())
    doCreateMenu(menu, item->childrenBegin(), item->childrenEnd());

  YMGAMenuBar::addItem(yitem);
}

YMGAGMenuBar::~YMGAGMenuBar()
{
  delete d;
}



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
#include <yui/gtk/ygtkratiobox.h>
#include <string.h>
#include <yui/mga/YMGAMenuItem.h>
#include <YTable.h>

#include <yui/gtk/YGDialog.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmenubar.h>
#include <gtk/gtk.h>
#include "YMGAGMenuBar.h"
#include <boost/filesystem.hpp>

#define YGTK_VBOX_NEW(arg) gtk_box_new(GTK_ORIENTATION_VERTICAL,arg)
#define YGTK_HBOX_NEW(arg) gtk_box_new(GTK_ORIENTATION_HORIZONTAL,arg)

typedef std::map<YItem*, GtkWidget*> MenuEntryMap;
typedef std::pair<YItem*, GtkWidget*> MenuEntryPair;

typedef std::map<YItem*, gulong> MenuCBMap;
typedef std::pair<YItem*, gulong> MenuCBPair;


struct YMGAGMenuBar::Private
{
  GtkWidget *menubar;

  MenuEntryMap menu_entry;
  MenuCBMap menu_cb;

  std::vector<GtkWidget*> menu_widgets;
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

static void selected_menuitem(GtkMenuItem *, YItem *item)
{
  YGUI::ui()->sendEvent (new YMenuEvent (item));
}

void YMGAGMenuBar::doCreateMenu (GtkWidget *menu, YItemIterator begin, YItemIterator end)
{
  for (YItemIterator it = begin; it != end; it++)
  {
    YMenuSeparator *separator = dynamic_cast<YMenuSeparator *>(*it);
    if (separator)
    {
      GtkWidget *sep = gtk_separator_menu_item_new();
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);
      gtk_widget_show(sep);
    }
    else
    {
      GtkWidget *entry;
      YItem * yitem = *it;
      std::string action_name = YGUtils::mapKBAccel (yitem->label());
      if (yitem->hasIconName())
      {
        GtkIconTheme * theme = gtk_icon_theme_get_default();
        std::string ico = boost::filesystem::path(yitem->iconName()).stem().c_str();
        GtkWidget *icon;
        GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
        if (gtk_icon_theme_has_icon (theme, ico.c_str()))
        {
          icon = gtk_image_new_from_icon_name (ico.c_str(), GTK_ICON_SIZE_MENU);
        }
        else
        {
          icon = gtk_image_new_from_file(yitem->iconName().c_str());
        }
        GtkWidget *label = gtk_label_new (action_name.c_str());
        entry = gtk_menu_item_new ();

        gtk_container_add (GTK_CONTAINER (box), icon);
        gtk_container_add (GTK_CONTAINER (box), label);

        gtk_label_set_use_underline (GTK_LABEL (label), TRUE);
        gtk_label_set_xalign (GTK_LABEL (label), 0.0);

        gtk_container_add (GTK_CONTAINER (entry), box);
      }
      else
      {
        entry = gtk_menu_item_new_with_mnemonic (action_name.c_str());
      }
      d->menu_entry.insert(MenuEntryPair(*it, entry));


      YMGAMenuItem *menuItem = dynamic_cast<YMGAMenuItem *>(*it);
      if (menuItem)
      {
        gtk_widget_set_sensitive(entry, menuItem->enabled() ? gtk_true() : gtk_false());

        yuiDebug() << menuItem->label() << " enabled: " << menuItem->enabled() << " hidden:" << menuItem->hidden() << std::endl;
      }
      if ((*it)->hasChildren()) {
        GtkWidget *submenu = gtk_menu_new();

        gtk_menu_item_set_submenu(GTK_MENU_ITEM(entry), submenu);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
        gtk_widget_show_all(entry);

        doCreateMenu (submenu, (*it)->childrenBegin(), (*it)->childrenEnd());

      }
      else
      {
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), entry);
        gtk_widget_show_all (entry);

        gulong id = g_signal_connect (G_OBJECT (entry), "activate",
                          G_CALLBACK (selected_menuitem), *it);

        d->menu_cb.insert(MenuCBPair(*it, id));
      }
      if (menuItem)
        if (menuItem->hidden())
          gtk_widget_hide(entry);
    }
  }
}


void YMGAGMenuBar::addItem(YItem* yitem)
{
  YMenuItem * item = dynamic_cast<YMenuItem *> ( yitem );
  YUI_CHECK_PTR ( item );
  yuiDebug() << item->label() << std::endl;

  // TODO icon from item
  GtkWidget *menu = gtk_menu_new();


  std::string lbl = YGUtils::mapKBAccel (item->label());
  GtkWidget *menu_entry = gtk_menu_item_new_with_mnemonic(lbl.c_str());

  d->menu_widgets.push_back(menu_entry);

  d->menu_entry.insert(MenuEntryPair(yitem, menu_entry));
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_entry), menu);
  gtk_widget_show(menu_entry);
  gtk_menu_shell_append (GTK_MENU_SHELL (d->menubar), menu_entry);

  YMGAMenuItem *menuItem = dynamic_cast<YMGAMenuItem *>(yitem);
  if (menuItem)
  {
    yuiDebug() << menuItem->label() << " enabled: " << menuItem->enabled() << " hidden:" << menuItem->hidden() << std::endl;
    gtk_widget_set_sensitive(menu_entry, menuItem->enabled() ? gtk_true() : gtk_false());
  }
  if (item->hasChildren())
    doCreateMenu(menu, item->childrenBegin(), item->childrenEnd());

  if (menuItem)
      if (menuItem->hidden())
        gtk_widget_hide(menu_entry);


  YMGAMenuBar::addItem(yitem);
}

void YMGAGMenuBar::enableItem(YItem* menu_item, bool enable)
{
  YMGAMenuBar::enableItem(menu_item, enable);

  auto search = d->menu_entry.find( menu_item );
  if (search != d->menu_entry.end())
  {
    GtkWidget * menu_entry = search->second;
    gtk_widget_set_sensitive(menu_entry, enable ? gtk_true() : gtk_false());
  }
  else
  {
    yuiError() << menu_item->label() << " not found" << std::endl;
  }
}

void YMGAGMenuBar::hideItem(YItem* menu_item, bool invisible)
{
  YMGAMenuBar::hideItem(menu_item, invisible);

  auto search = d->menu_entry.find( menu_item );
  if (search != d->menu_entry.end())
  {
    GtkWidget * menu_entry = search->second;
    gtk_widget_set_visible(menu_entry, invisible ? gtk_false() : gtk_true());
//     if (!invisible)
//     {
//       int min_width = this->getMinSize (YD_HORIZ);
//       int min_height = this->getMinSize (YD_VERT);
//       ygtk_adj_size_set_min(YGTK_ADJ_SIZE(getLayout()), min_width + menu_item->label().size(), min_height);
//
//       gtk_widget_queue_resize(getWidget());
//     }

  }
  else
  {
    yuiError() << menu_item->label() << " not found" << std::endl;
  }
}

void YMGAGMenuBar::deleteAllItems()
{

  for (MenuCBMap::iterator it=d->menu_cb.begin(); it!=d->menu_cb.end(); ++it)
  {
    auto search = d->menu_entry.find( it->first );
    if (search != d->menu_entry.end())
      g_signal_handler_disconnect (search->second, it->second);
  }


  for (GtkWidget *m: d->menu_widgets)
  {
    gtk_widget_destroy(m);
  }

//   for (MenuEntryMap::iterator it=d->menu_entry.begin(); it!=d->menu_entry.end(); ++it)
//   {
//     gtk_widget_destroy(it->second);
//   }

  d->menu_widgets.clear();
  d->menu_cb.clear();
  d->menu_entry.clear();

  YSelectionWidget::deleteAllItems();
}


YMGAGMenuBar::~YMGAGMenuBar()
{
  d->menu_cb.clear();
  d->menu_entry.clear();

  delete d;
}



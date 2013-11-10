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

  File:       YMGA_GCBTable.h

  Author:     Angelo Naselli <anaselli@linux.it>

/-*/

#ifndef YMGA_GCBTable_h
#define YMGA_GCBTable_h


#include <yui/mga/YMGA_CBTable.h>
#include <yui/gtk/ygtktreeview.h>


#include <yui/gtk/YGSelectionStore.h>
#include <yui/gtk/YGWidget.h>

/* YMGA_GTreeView is YGTreeView into gtk implementation that unfortunately is not usable
   TODO if needed by other classes move to a new header file
 */

class YMGA_GTreeView : public YGScrolledWidget, public YGSelectionStore
{
protected:
    guint m_blockTimeout;
    int markColumn;
    GtkWidget *m_count;

public:
    YMGA_GTreeView (YWidget *ywidget, YWidget *parent, const std::string &label, bool tree);

    virtual ~YMGA_GTreeView();

    inline GtkTreeView *getView()
    {
        return GTK_TREE_VIEW (getWidget());
    }
    inline GtkTreeSelection *getSelection()
    {
        return gtk_tree_view_get_selection (getView());
    }

    void addTextColumn (int iconCol, int textCol);
    void addTextColumn (const std::string &header, YAlignmentType align, int icon_col, int text_col);

    void addCheckColumn (int check_col);

    void readModel();

    void addCountWidget (YWidget *yparent);

    void syncCount();

    void focusItem (YItem *item, bool select);

    void unfocusAllItems();

    void unmarkAll();

    YItem *getFocusItem();

    virtual bool _immediateMode() {
        return true;
    }
    virtual bool _shrinkable() {
        return false;
    }
    virtual bool _recursiveSelection() {
        return false;
    }

    void setMark (GtkTreeIter *iter, YItem *yitem, gint column, bool state, bool recursive);

    void toggleMark (GtkTreePath *path, gint column);

    virtual unsigned int getMinSize (YUIDimension dim);

protected:
    static gboolean block_selected_timeout_cb (gpointer data);

    void blockSelected();

    static void block_init_cb (GtkWidget *widget, YMGA_GTreeView *pThis);

    static bool all_marked (GtkTreeModel *model, GtkTreeIter *iter, int mark_col);

    static void inconsistent_mark_cb (GtkTreeViewColumn *column,
                                      GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data);

    static void selection_changed_cb (GtkTreeSelection *selection, YMGA_GTreeView *pThis);

    static void activated_cb (GtkTreeView *tree_view, GtkTreePath *path,
                              GtkTreeViewColumn *column, YMGA_GTreeView* pThis);

    static void toggled_cb (GtkCellRendererToggle *renderer, gchar *path_str,
                            YMGA_GTreeView *pThis);
    static void right_click_cb (YGtkTreeView *view, gboolean outreach, YMGA_GTreeView *pThis);

};


class YMGA_GCBTable : public YMGA_CBTable, public YMGA_GTreeView
{
public:
    YMGA_GCBTable (YWidget *parent, YTableHeader *headers, YTableMode mode);

    void setSortable (bool sortable);

    void setCell (GtkTreeIter *iter, int column, const YTableCell *cell);

    // YGTreeView
    virtual bool _immediateMode();

    // YTable
    virtual void setKeepSorting (bool keepSorting);

    virtual void cellChanged (const YTableCell *cell);

    // YGSelectionStore
    void doAddItem (YItem *_item);

    void doSelectItem (YItem *item, bool select);

    void doDeselectAllItems();

    // callbacks
    static void activateButton (YWidget *button);

    static void hack_right_click_cb (YGtkTreeView *view, gboolean outreach, YMGA_GCBTable *pThis);

    static gboolean key_press_event_cb (GtkWidget *widget, GdkEventKey *event, YMGA_GCBTable *pThis);

    static gint tree_sort_cb (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer _index);

//     YGLABEL_WIDGET_IMPL (YTable)
//     YGSELECTION_WIDGET_IMPL (YTable)
    YGLABEL_WIDGET_IMPL (YMGA_CBTable)
    YGSELECTION_WIDGET_IMPL (YMGA_CBTable)
};




#endif // YQLabel_h

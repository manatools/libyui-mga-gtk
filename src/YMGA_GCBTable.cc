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

  File:	      YMGA_GCBTable.cc

  Author:     Angelo Naselli <anaselli@linux.it>

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

#include "YMGA_GCBTable.h"

//**** YMGA_GTreeView implementation

YMGA_GTreeView::YMGA_GTreeView (YWidget *ywidget, YWidget *parent, const std::string &label, bool tree)
    : YGScrolledWidget (ywidget, parent, label, YD_VERT, YGTK_TYPE_TREE_VIEW, NULL),
      YGSelectionStore (tree)
{
    gtk_tree_view_set_headers_visible (getView(), FALSE);

    /* Yast tools expect the user to be unable to un-select the row. They
       generally don't check to see if the returned value is -1. So, just
       disallow un-selection. */
    gtk_tree_selection_set_mode (getSelection(), GTK_SELECTION_BROWSE);

    connect (getSelection(), "changed", G_CALLBACK (selection_changed_cb), this);
    connect (getWidget(), "row-activated", G_CALLBACK (activated_cb), this);
    connect (getWidget(), "right-click", G_CALLBACK (right_click_cb), this);

    m_blockTimeout = 0;  // GtkTreeSelection idiotically fires when showing widget
    markColumn = -1;
    m_count = NULL;
    blockSelected();
    g_signal_connect (getWidget(), "map", G_CALLBACK (block_init_cb), this);
}

YMGA_GTreeView::~YMGA_GTreeView()
{
    if (m_blockTimeout) g_source_remove (m_blockTimeout);
}

void YMGA_GTreeView::addTextColumn (int iconCol, int textCol)
{
    addTextColumn ("", YAlignUnchanged, iconCol, textCol);
}

void YMGA_GTreeView::addTextColumn (const std::string &header, YAlignmentType align, int icon_col, int text_col)
{
    gfloat xalign = -1;
    switch (align) {
    case YAlignBegin:
        xalign = 0.0;
        break;
    case YAlignCenter:
        xalign = 0.5;
        break;
    case YAlignEnd:
        xalign = 1.0;
        break;
    case YAlignUnchanged:
        break;
    }

    GtkTreeViewColumn *column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title (column, header.c_str());

    GtkCellRenderer *renderer;
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, "pixbuf", icon_col, NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", text_col, NULL);
    if (xalign != -1)
        g_object_set (renderer, "xalign", xalign, NULL);

    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (getView(), column);
    if (gtk_tree_view_get_search_column (getView()) == -1)
        gtk_tree_view_set_search_column (getView(), text_col);
}

void YMGA_GTreeView::addCheckColumn (int check_col)
{
    GtkCellRenderer *renderer = gtk_cell_renderer_toggle_new();
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (check_col));
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (
                                    NULL, renderer, "active", check_col, NULL);
    gtk_tree_view_column_set_cell_data_func (column, renderer, inconsistent_mark_cb, this, NULL);
    g_signal_connect (G_OBJECT (renderer), "toggled",
                      G_CALLBACK (toggled_cb), this);

    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (getView(), column);
    if (markColumn == -1)
        markColumn = check_col;
}

void YMGA_GTreeView::readModel()
{
    gtk_tree_view_set_model (getView(), getModel());
}

void YMGA_GTreeView::addCountWidget (YWidget *yparent)
{
    bool mainWidget = !yparent || !strcmp (yparent->widgetClass(), "YVBox") || !strcmp (yparent->widgetClass(), "YReplacePoint");
    if (mainWidget) {
        m_count = gtk_label_new ("0");
        GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
        gtk_box_set_homogeneous (GTK_BOX (hbox), FALSE);

        GtkWidget *label = gtk_label_new (_("Total selected:"));
        //gtk_box_pack_start (GTK_BOX (hbox), gtk_event_box_new(), TRUE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), m_count, FALSE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (YGWidget::getWidget()), hbox, FALSE, TRUE, 0);
        gtk_widget_show_all (hbox);
    }
}

void YMGA_GTreeView::syncCount()
{
    if (!m_count) return;

    struct inner {
        static gboolean foreach (
            GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer _pThis)
        {
            YMGA_GTreeView *pThis = (YMGA_GTreeView *) _pThis;
            gboolean mark;
            gtk_tree_model_get (model, iter, pThis->markColumn, &mark, -1);
            if (mark) {
                int count = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (model), "count"));
                g_object_set_data (G_OBJECT (model), "count", GINT_TO_POINTER (count+1));
            }
            return FALSE;
        }
    };

    GtkTreeModel *model = getModel();
    g_object_set_data (G_OBJECT (model), "count", 0);
    gtk_tree_model_foreach (model, inner::foreach, this);

    int count = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (model), "count"));
    gchar *str = g_strdup_printf ("%d", count);
    gtk_label_set_text (GTK_LABEL (m_count), str);
    g_free (str);
}

void YMGA_GTreeView::focusItem (YItem *item, bool select)
{
    GtkTreeIter iter;
    getTreeIter (item, &iter);
    blockSelected();

    if (select) {
        GtkTreePath *path = gtk_tree_model_get_path (getModel(), &iter);
        gtk_tree_view_expand_to_path (getView(), path);

        if (gtk_tree_selection_get_mode (getSelection()) != GTK_SELECTION_MULTIPLE)
            gtk_tree_view_scroll_to_cell (getView(), path, NULL, TRUE, 0.5, 0);
        gtk_tree_path_free (path);

        gtk_tree_selection_select_iter (getSelection(), &iter);
    }
    else
        gtk_tree_selection_unselect_iter (getSelection(), &iter);
    
    
    setRowMark (&iter, markColumn, select);
}

void YMGA_GTreeView::unfocusAllItems()
{
    blockSelected();
    gtk_tree_selection_unselect_all (getSelection());
}

void YMGA_GTreeView::unmarkAll()
{
    struct inner {
        static gboolean foreach_unmark (
            GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer _pThis)
        {
            YMGA_GTreeView *pThis = (YMGA_GTreeView *) _pThis;
            pThis->setRowMark (iter, pThis->markColumn, FALSE);
            return FALSE;
        }
    };

    gtk_tree_model_foreach (getModel(), inner::foreach_unmark, this);
}

YItem *YMGA_GTreeView::getFocusItem()
{
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected (getSelection(), NULL, &iter))
        return getYItem (&iter);
    return NULL;
}


void YMGA_GTreeView::setMark (GtkTreeIter *iter, YItem *yitem, gint column, bool state, bool recursive)
{
    setRowMark (iter, column, state);
    yitem->setSelected (state);

    if (recursive)
        for (YItemConstIterator it = yitem->childrenBegin();
                it != yitem->childrenEnd(); it++) {
            GtkTreeIter _iter;
            getTreeIter (*it, &_iter);
            setMark (&_iter, *it, column, state, true);
        }
}

void YMGA_GTreeView::toggleMark (GtkTreePath *path, gint column)
{
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter (getModel(), &iter, path))
        return;
    gboolean state;
    gtk_tree_model_get (getModel(), &iter, column, &state, -1);
    state = !state;

    YItem *yitem = getYItem (&iter);
    YTableItem *pYTableItem = dynamic_cast<YTableItem*>(yitem);
    if (pYTableItem)
    {
        YMGA_CBTable * pTable = dynamic_cast<YMGA_CBTable*>(this);
        if (pTable)
        {
            if ( (pTable->selectionMode() == YTableMode::YTableCheckBoxOnFirstColumn &&  column == 0)
                    ||
                    (pTable->selectionMode() == YTableMode::YTableCheckBoxOnLastColumn  &&  column == pTable->columns()*3 ))
            {
                setRowMark (&iter, column, state);
                yitem->setSelected (state);
                pTable->setChangedItem(pYTableItem);
                emitEvent (YEvent::ValueChanged);
            }

        }
    }
    else
    {
        setMark (&iter, yitem, column, state, _recursiveSelection());
        syncCount();
        emitEvent (YEvent::ValueChanged);
    }
}

unsigned int YMGA_GTreeView::getMinSize (YUIDimension dim)
{
    if (dim == YD_VERT)
        return YGUtils::getCharsHeight (getWidget(), _shrinkable() ? 2 : 5);
    return 80;
}

gboolean YMGA_GTreeView::block_selected_timeout_cb (gpointer data)
{
    YMGA_GTreeView *pThis = (YMGA_GTreeView *) data;
    pThis->m_blockTimeout = 0;
    return FALSE;
}

void YMGA_GTreeView::blockSelected()
{   // GtkTreeSelection only fires when idle; so set a timeout
    if (m_blockTimeout) g_source_remove (m_blockTimeout);
    m_blockTimeout = g_timeout_add_full (G_PRIORITY_LOW, 50, block_selected_timeout_cb, this, NULL);
}

void YMGA_GTreeView::block_init_cb (GtkWidget *widget, YMGA_GTreeView *pThis)
{
    pThis->blockSelected();
}

// callbacks

bool YMGA_GTreeView::all_marked (GtkTreeModel *model, GtkTreeIter *iter, int mark_col)
{
    gboolean marked;
    GtkTreeIter child_iter;
    if (gtk_tree_model_iter_children (model, &child_iter, iter))
        do {
            gtk_tree_model_get (model, &child_iter, mark_col, &marked, -1);
            if (!marked) return false;
            all_marked (model, &child_iter, mark_col);
        } while (gtk_tree_model_iter_next (model, &child_iter));
    return true;
}

void YMGA_GTreeView::inconsistent_mark_cb (GtkTreeViewColumn *column,
        GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{   // used for trees -- show inconsistent if one node is check but another isn't
    YMGA_GTreeView *pThis = (YMGA_GTreeView *) data;
    gboolean marked;
    gtk_tree_model_get (model, iter, pThis->markColumn, &marked, -1);
    gboolean consistent = !marked || all_marked (model, iter, pThis->markColumn);
    g_object_set (G_OBJECT (cell), "inconsistent", !consistent, NULL);
}

void YMGA_GTreeView::selection_changed_cb (GtkTreeSelection *selection, YMGA_GTreeView *pThis)
{
    struct inner {
        static gboolean foreach_sync_select (
            GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer _pThis)
        {
            YMGA_GTreeView *pThis = (YMGA_GTreeView *) _pThis;
            GtkTreeSelection *selection = pThis->getSelection();
            bool sel = gtk_tree_selection_iter_is_selected (selection, iter);
            pThis->getYItem (iter)->setSelected (sel);
            return FALSE;
        }
    };

    if (pThis->m_blockTimeout) return;
    if (pThis->markColumn == -1)
        gtk_tree_model_foreach (pThis->getModel(), inner::foreach_sync_select, pThis);
    if (pThis->_immediateMode())
        pThis->emitEvent (YEvent::SelectionChanged, IF_NOT_PENDING_EVENT);
}

void YMGA_GTreeView::activated_cb (GtkTreeView *tree_view, GtkTreePath *path,
                                   GtkTreeViewColumn *column, YMGA_GTreeView* pThis)
{
    if (pThis->markColumn >= 0)
    {
        YTable *pTable = dynamic_cast<YTable*>(pThis);
        if (pTable)
        {
            GtkTreeViewColumn* col = gtk_tree_view_get_column (pThis->getView(), pThis->markColumn);
            if (col == column)
                pThis->toggleMark (path, pThis->markColumn);
            else
                pThis->emitEvent (YEvent::Activated);
        }
        else
            pThis->toggleMark (path, pThis->markColumn);
    }
    else {
        // for tree - expand/collpase double-clicked rows
        if (gtk_tree_view_row_expanded (tree_view, path))
            gtk_tree_view_collapse_row (tree_view, path);
        else
            gtk_tree_view_expand_row (tree_view, path, FALSE);

        pThis->emitEvent (YEvent::Activated);
    }
}

void YMGA_GTreeView::toggled_cb (GtkCellRendererToggle *renderer, gchar *path_str,
                                 YMGA_GTreeView *pThis)
{
    GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
    gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (renderer), "column"));
    pThis->toggleMark (path, column);
    gtk_tree_path_free (path);

    // un/marking a sub-node can cause changes of "inconsistency"
    if (gtk_tree_path_get_depth (path) >= 2)
        gtk_widget_queue_draw (pThis->getWidget());
}

void YMGA_GTreeView::right_click_cb (YGtkTreeView *view, gboolean outreach, YMGA_GTreeView *pThis)
{
    pThis->emitEvent (YEvent::ContextMenuActivated);
}


//**** YMGA_GCBTable implementation

YMGA_GCBTable::YMGA_GCBTable (YWidget *parent, YTableHeader *headers, YTableMode mode)
    : YMGA_CBTable (NULL, headers, mode),
      YMGA_GTreeView (this, parent, std::string(), false)
{
    gtk_tree_view_set_headers_visible (getView(), TRUE);
    gtk_tree_view_set_rules_hint (getView(), columns() > 1);
    ygtk_tree_view_set_empty_text (YGTK_TREE_VIEW (getView()), _("No entries."));
    if ( mode == YTableMultiSelection )
        gtk_tree_selection_set_mode (getSelection(), GTK_SELECTION_MULTIPLE);
    int columnNumber = columns();
    int columnOffset = 0;

    yuiMilestone() << " Slection mode " << mode <<  std::endl;

    if (mode == YTableCheckBoxOnFirstColumn)
    {
        columnNumber += 1;
        columnOffset = 1;
    }
    else if ( mode == YTableCheckBoxOnLastColumn)
        columnNumber += 1;
    GType types [columnNumber*3];
    for (int i = 0; i < columnNumber; i++) {
        int t = i*3;
        types[t+0] = G_TYPE_BOOLEAN;
        types[t+1] = GDK_TYPE_PIXBUF;
        types[t+2] = G_TYPE_STRING;
        if ( (i==0 && mode == YTableCheckBoxOnFirstColumn) ||
                (i == columnNumber-1 && mode == YTableCheckBoxOnLastColumn))
            addCheckColumn(t);
        else
            addTextColumn  (header(i-columnOffset), alignment (i-columnOffset), t+1, t+2);
    }
    createStore (columnNumber*3, types);
    readModel();
    if (!keepSorting())
        setSortable (true);

    yuiMilestone() << " columns " << columns() << " tot " << columnNumber <<  std::endl;

    // if last col is aligned: add some dummy so that it doesn't expand.
    YAlignmentType lastAlign = alignment (columnNumber-1);
    if (lastAlign == YAlignCenter || lastAlign == YAlignEnd)
        gtk_tree_view_append_column (getView(), gtk_tree_view_column_new());

    g_signal_connect (getWidget(), "key-press-event", G_CALLBACK (key_press_event_cb), this);
}

void YMGA_GCBTable::setSortable (bool sortable)
{
    if (!sortable && !gtk_widget_get_realized (getWidget()))
        return;
    int n = 0;
    GList *columns = gtk_tree_view_get_columns (getView());
    for (GList *i = columns; i; i = i->next, n++) {
        GtkTreeViewColumn *column = (GtkTreeViewColumn *) i->data;
        if (n >= YMGA_GCBTable::columns())
            break;
        if (sortable) {
            // anaselli it was (n*2)+1
            int index = (n*3)+1;
            if (!sortable)
                index = -1;
            gtk_tree_sortable_set_sort_func (
                GTK_TREE_SORTABLE (getModel()), index, tree_sort_cb,
                GINT_TO_POINTER (index), NULL);
            gtk_tree_view_column_set_sort_column_id (column, index);
        }
        else
            gtk_tree_view_column_set_sort_column_id (column, -1);
    }
    g_list_free (columns);
}

void YMGA_GCBTable::setCell (GtkTreeIter *iter, int column, const YTableCell *cell)
{
    if (!cell) return;
    std::string label (cell->label());
    if (label == "X")
        label = YUI::app()->glyph (YUIGlyph_CheckMark);

    int index = column * 3;
    setRowText (iter, index+1, cell->iconName(), index+2, label, this);
}

// YGTreeView

bool YMGA_GCBTable::_immediateMode() {
    return immediateMode();
}

// YTable

void YMGA_GCBTable::setKeepSorting (bool keepSorting)
{
    YMGA_CBTable::setKeepSorting (keepSorting);
    setSortable (!keepSorting);
    if (!keepSorting) {
        GtkTreeViewColumn *column = gtk_tree_view_get_column (getView(), 0);
        if (column)
            gtk_tree_view_column_clicked (column);
    }
}

void YMGA_GCBTable::cellChanged (const YTableCell *cell)
{
    GtkTreeIter iter;
    getTreeIter (cell->parent(), &iter);
    setCell (&iter, cell->column(), cell);
}

void YMGA_GCBTable::doAddItem (YItem *_item)
{
    YTableItem *item = dynamic_cast <YTableItem *> (_item);
    if (item) {
        GtkTreeIter iter;
        addRow (item, &iter);
        int i = 0;
        int column_offset = 0;
        if (selectionMode() == YTableMode::YTableCheckBoxOnFirstColumn )
        {
            column_offset=1;
            setRowMark(&iter, i++, item->selected());
        }
        for (YTableCellIterator it = item->cellsBegin();
                it != item->cellsEnd(); it++)
        {
            if ((i-column_offset) > columns())
            {
                yuiWarning() << "Item contains too many columns, current is " << i-column_offset
                             << " but only " << columns() << " columns are configured" << std::endl;
            }
            else
                setCell (&iter, i++, *it);
        }
        if (selectionMode() == YTableMode::YTableCheckBoxOnLastColumn )
        {
            yuiMilestone() << " columns " << columns() << std::endl;
            setRowMark(&iter, columns()*3, item->selected());
        }
        if (item->selected())
            focusItem (item, true);
    }
    else
        yuiError() << "Can only add YTableItems to a YTable.\n";
}

void YMGA_GCBTable::doSelectItem (YItem *item, bool select)
{
    focusItem (item, select);
}

void YMGA_GCBTable::doDeselectAllItems()
{
    unfocusAllItems();
}

// callbacks

void YMGA_GCBTable::activateButton (YWidget *button)
{
    YWidgetEvent *event = new YWidgetEvent (button, YEvent::Activated);
    YGUI::ui()->sendEvent (event);
}

void YMGA_GCBTable::hack_right_click_cb (YGtkTreeView* view, gboolean outreach, YMGA_GCBTable* pThis)
{
    if (pThis->notifyContextMenu())
        return YMGA_GTreeView::right_click_cb (view, outreach, pThis);

    // If no context menu is specified, hack one ;-)

    struct inner {
        static void key_activate_cb (GtkMenuItem *item, YWidget *button)
        {
            activateButton (button);
        }
        static void appendItem (GtkWidget *menu, const gchar *stock, int key)
        {
            YWidget *button = YGDialog::currentDialog()->getFunctionWidget (key);
            if (button) {
                GtkWidget *item;
                item = gtk_menu_item_new_with_mnemonic (stock);
                gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
                g_signal_connect (G_OBJECT (item), "activate",
                                  G_CALLBACK (key_activate_cb), button);
            }
        }
    };

    GtkWidget *menu = gtk_menu_new();
    YGDialog *dialog = YGDialog::currentDialog();
    if (dialog->getClassWidgets ("YTable").size() == 1) {
        // if more than one table exists, function keys would be ambiguous
        if (dialog->getClassWidgets ("YTable").size() == 1) {
                        // if more than one table exists, function keys would be ambiguous
                        if (outreach) {
                                if (dialog->getFunctionWidget(3))
                                        inner::appendItem (menu, "list-add", 3);
                        }
                        else {
                                if (dialog->getFunctionWidget(4))
                                        inner::appendItem (menu, "edit-cut", 4);
                                if (dialog->getFunctionWidget(5))
                                        inner::appendItem (menu, "list-remove", 5);
                        }
                }
    }

    menu = ygtk_tree_view_append_show_columns_item (YGTK_TREE_VIEW (view), menu);
    ygtk_tree_view_popup_menu (view, menu);
}

gboolean YMGA_GCBTable::key_press_event_cb (GtkWidget* widget, GdkEventKey* event, YMGA_GCBTable* pThis)
{
    if (event->keyval == GDK_KEY_Delete) {
        YWidget *button = YGDialog::currentDialog()->getFunctionWidget (5);
        if (button)
            activateButton (button);
        else
            gtk_widget_error_bell (widget);
        return TRUE;
    }
    return FALSE;
}

gint YMGA_GCBTable::tree_sort_cb (
    GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer _index)
{
    int index = GPOINTER_TO_INT (_index);
    gchar *str_a, *str_b;
    gtk_tree_model_get (model, a, index, &str_a, -1);
    gtk_tree_model_get (model, b, index, &str_b, -1);
    if (!str_a) str_a = g_strdup ("");
    if (!str_b) str_b = g_strdup ("");
    int ret = strcmp (str_a, str_b);
    g_free (str_a);
    g_free (str_b);
    return ret;
}


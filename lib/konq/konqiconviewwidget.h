/*  This file is part of the KDE project
    Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __konq_iconviewwidget_h__
#define __konq_iconviewwidget_h__

#include <kiconloader.h>
#include <kiconview.h>
#include <kurl.h>
#include <konqfileitem.h>

class KonqFMSettings;
class KFileIVI;
namespace KIO { class Job; }

/**
 * A file-aware icon view, implementing drag'n'drop, KDE icon sizes,
 * user settings, ...
 * Used by kdesktop and konq_iconview
 */
class KonqIconViewWidget : public KIconView
{
    Q_OBJECT
    Q_PROPERTY( bool sortDirectoriesFirst READ sortDirectoriesFirst WRITE setSortDirectoriesFirst );
public:
    /**
     * Constructor
     * @param settings An instance of KonqFMSettings, see static methods in konqsettings.h
     */
    KonqIconViewWidget( QWidget *parent = 0L, const char *name = 0L, WFlags f = 0 );
    virtual ~KonqIconViewWidget() {}

    void initConfig();

    /**
     * Sets the icons of all items, and stores the @p size
     */
    void setIcons( KIconLoader::Size size );

    /**
     * Called on databaseChanged
     */
    void refreshMimeTypes();

    KIconLoader::Size size() { return m_size; }

    void setImagePreviewAllowed( bool b );

    void setURL ( const KURL & kurl ) { m_url = kurl; }
    const KURL & url() { return m_url; }
    void setRootItem ( const KonqFileItem * item ) { m_rootItem = item; }

    /** Made public for konq_iconview (copy) */
    virtual QDragObject *dragObject();

    /**
     * Get list of selected KonqFileItems
     */
    KonqFileItemList selectedFileItems();

    void setItemFont( const QFont &f );
    void setItemColor( const QColor &c );
    QColor itemColor() const;

    virtual void cutSelection();
    virtual void copySelection();
    virtual void pasteSelection( bool move );
    virtual KURL::List selectedUrls();

    bool sortDirectoriesFirst() const;
    void setSortDirectoriesFirst( bool b );

public slots:
    /**
     * Checks the new selection and emits enableAction() signals
     */
    virtual void slotSelectionChanged();

signals:
    /**
     * For cut/copy/paste/move/delete (see kparts/browserextension.h)
     */
    void enableAction( const char * name, bool enabled );
    void viewportAdjusted();

protected slots:

    virtual void slotViewportScrolled(int);

    virtual void slotDropped( QDropEvent *e, const QValueList<QIconDragItem> & );
    /** connect each item to this */
    virtual void slotDropItem( KFileIVI *item, QDropEvent *e );

    void slotResult( KIO::Job * job );

protected:
    virtual void drawBackground( QPainter *p, const QRect &r );
    virtual void viewportResizeEvent(QResizeEvent *);

    KURL m_url;
    const KonqFileItem * m_rootItem;

    KIconLoader::Size m_size;
    bool m_bImagePreviewAllowed;

    /** Konqueror settings */
    KonqFMSettings * m_pSettings;

    QColor iColor;

    bool m_bSortDirsFirst;
};

#endif

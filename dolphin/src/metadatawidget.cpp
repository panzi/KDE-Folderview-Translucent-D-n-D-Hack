/***************************************************************************
 *   Copyright (C) 2007 by Sebastian Trueg <trueg@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "metadatawidget.h"

#include <config-kmetadata.h>

#include <klocale.h>

#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QTextEdit>

#ifdef HAVE_KMETADATA
#include <kmetadata/kmetadatatagwidget.h>
#include <kmetadata/resourcemanager.h>
#include <kmetadata/resource.h>
#include <kmetadata/variant.h>
#include <kmetadata/kratingwidget.h>
#endif

// FIXME: these should be replaced by using KMetaData::File once it is available again
static const char* s_nfoFile = "http://ont.semanticdesktop.org/2007/03/22/nfo#File";
static const char* s_nfoFileUrl = "http://ont.semanticdesktop.org/2007/03/22/nfo#fileUrl";


bool MetaDataWidget::metaDataAvailable()
{
#ifdef HAVE_KMETADATA
    return !Nepomuk::KMetaData::ResourceManager::instance()->init();
#else
    return false;
#endif
}


class MetaDataWidget::Private
{
public:
#ifdef HAVE_KMETADATA
    void loadComment(const QString& comment);

    QMap<KUrl, Nepomuk::KMetaData::Resource> files;

    QTextEdit* editComment;
    KRatingWidget* ratingWidget;
    Nepomuk::KMetaData::TagWidget* tagWidget;
#endif
};

#ifdef HAVE_KMETADATA
void MetaDataWidget::Private::loadComment(const QString& comment)
{
    editComment->blockSignals(true);
    if (comment.isEmpty()) {
        editComment->setFontItalic(true);
        editComment->setText(i18n("Click to add comment..."));
    } else {
        editComment->setFontItalic(false);
        editComment->setText(comment);
    }
    editComment->blockSignals(false);
}
#endif


MetaDataWidget::MetaDataWidget(QWidget* parent) :
    QWidget(parent)
{
#ifdef HAVE_KMETADATA
    d = new Private;
    d->editComment = new QTextEdit(this);
    d->ratingWidget = new KRatingWidget(this);
    d->tagWidget = new Nepomuk::KMetaData::TagWidget(this);
    connect(d->ratingWidget, SIGNAL(ratingChanged(unsigned int)), this, SLOT(slotRatingChanged(unsigned int)));
    connect(d->editComment, SIGNAL(textChanged()), this, SLOT(slotCommentChanged()));

    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->setMargin(0);
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel(i18n("Rating:"), this));
    hbox->addStretch(1);
    hbox->addWidget(d->ratingWidget);
    lay->addLayout(hbox);
    lay->addWidget(d->editComment);
    hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel(i18n("Tags:"), this));
    hbox->addWidget(d->tagWidget, 1);
    lay->addLayout(hbox);

    d->editComment->installEventFilter(this);
    d->editComment->viewport()->installEventFilter(this);
#else
    d = 0;
#endif
}


MetaDataWidget::~MetaDataWidget()
{
    delete d;
}


void MetaDataWidget::setFile(const KUrl& url)
{
    KUrl::List urls;
    urls.append( url );
    setFiles( urls );
}


void MetaDataWidget::setFiles(const KUrl::List& urls)
{
#ifdef HAVE_KMETADATA
    // FIXME: replace with KMetaData::File once we have it again
    d->files.clear();
    bool first = true;
    QList<Nepomuk::KMetaData::Resource> fileRes;
    Q_FOREACH( KUrl url, urls ) {
        Nepomuk::KMetaData::Resource file( url.url(), s_nfoFile );
//    file.setLocation(url.url());
        d->files.insert( url, file );
        fileRes.append( file );

       if ( !first &&
            d->ratingWidget->rating() != file.rating() ) {
           d->ratingWidget->setRating( 0 ); // reset rating
       }
       else if ( first ) {
           d->ratingWidget->setRating( file.rating() );
       }

       if ( !first &&
            d->editComment->toPlainText() != file.description() ) {
           d->loadComment( QString() );
       }
       else if ( first ) {
           d->loadComment( file.description() );
       }
       first = false;
    }
    d->tagWidget->setTaggedResources(fileRes);
#endif
}


void MetaDataWidget::slotCommentChanged()
{
#ifdef HAVE_KMETADATA
    for ( QMap<KUrl, Nepomuk::KMetaData::Resource>::iterator it = d->files.begin();
          it != d->files.end(); ++it ) {
        it.value().setDescription(d->editComment->toPlainText());
    }
#endif
}


void MetaDataWidget::slotRatingChanged(unsigned int rating)
{
#ifdef HAVE_KMETADATA
    for ( QMap<KUrl, Nepomuk::KMetaData::Resource>::iterator it = d->files.begin();
          it != d->files.end(); ++it ) {
        it.value().setRating(rating);
    }
#endif
}


bool MetaDataWidget::eventFilter(QObject* obj, QEvent* event)
{
#ifdef HAVE_KMETADATA
    if (obj == d->editComment->viewport() || obj == d->editComment) {
        if (event->type() == QEvent::FocusOut) {
            // make sure the info text is displayed again
            d->loadComment(d->editComment->toPlainText());
        } else if (event->type() == QEvent::FocusIn) {
            d->editComment->setFontItalic(false);
            if (!d->files.isEmpty() && d->files.begin().value().description().isEmpty()) {
                d->editComment->setText(QString());
            }
        }
    }
#endif

    return QWidget::eventFilter(obj, event);
}

#include "metadatawidget.moc"

/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
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
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef TUPPROJECT_H
#define TUPPROJECT_H

#include "tglobal.h"
#include "tupabstractserializable.h"
#include "tapplicationproperties.h"
#include "tuplibraryobject.h"

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <QSize>
#include <QDir>
#include <QGraphicsView>

class TupScene;
class TupLayer;
class TupFrame;
class TupProjectRequest;
class TupProjectResponse;
class QGraphicsItem;
class TupLibrary;

typedef QList<TupScene *> Scenes;

class TUPITUBE_EXPORT TupProject : public QObject, public TupAbstractSerializable
{
    Q_OBJECT

    public:

        enum Mode 
        {
            FRAMES_EDITION = 0,
            STATIC_BACKGROUND_EDITION,
            DYNAMIC_BACKGROUND_EDITION,
            NONE
        };

        TupProject(QObject *parent = 0);
        ~TupProject();

        void setProjectName(const QString &name);
        void setAuthor(const QString &author);
        void setBgColor(const QColor color);
        void setDescription(const QString &description);
        void setDimension(const QSize dimension);
        void setFPS(const int fps);	

        QString projectName() const;
        QString author() const;
        QColor bgColor() const;
        QString description() const;
        QSize dimension() const;
        int fps() const;

        // bool deleteDataDir(const QString &path);

        void setDataDir(const QString &path);
        QString dataDir() const;

        TupScene *sceneAt(int pos) const;

        int visualIndexOf(TupScene *scene) const;

        Scenes scenes() const;

        TupScene *createScene(QString name, int pos, bool loaded = false);
        void updateScene(int pos, TupScene *scene);
        bool restoreScene(int pos);
        bool removeScene(int pos);
        bool resetScene(int pos, const QString &newName);
        QString recoverScene(int pos) const;
        bool moveScene(int pos, int newPos);

        bool createSymbol(int type, const QString &name, const QByteArray &data, const QString &folder = QString());
        bool removeSymbol(const QString &name, TupLibraryObject::Type type);
        bool addFolder(const QString &name);
        bool removeFolder(const QString &name);

        bool removeSound(const QString &name);

        bool insertSymbolIntoFrame(TupProject::Mode spaceMode, const QString &name, int scene, int layer, int frame);
        bool removeSymbolFromFrame(const QString &name, TupLibraryObject::Type type);

        bool updateSymbolId(TupLibraryObject::Type type, const QString &oldId, const QString &newId);
        void reloadLibraryItem(TupLibraryObject::Type type, const QString &id, TupLibraryObject *object);

        void clear();
        void loadLibrary(const QString &filename);

        TupLibrary *library();
        void emitResponse(TupProjectResponse *response);

        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc) const;

        void setOpen(bool open);
        bool isOpen();
        int scenesCount() const;

        void updateSpaceContext(TupProject::Mode mode);
        TupProject::Mode spaceContext();

        void importLayer(int sceneIndex, const QString &layer);

    signals:
        void responsed(TupProjectResponse *response);

    private:
        struct Private;
        Private *const k;
};

#endif

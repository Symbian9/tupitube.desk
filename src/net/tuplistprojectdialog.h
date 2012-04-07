/***************************************************************************
 *   Project TUPI: Magia 2D                                                *
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
 *   the Free Software Foundation; either version 3 of the License, or     *
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

#ifndef TupLISTPROJECTDIALOG_H
#define TupLISTPROJECTDIALOG_H

#include "tapplicationproperties.h"

#include <QDialog>
#include <QTreeWidget>

/**
 * @author Jorge Cuadrado
*/

class QTreeWidgetItem;

class TupListProjectDialog : public QDialog
{
    Q_OBJECT

    public:
        TupListProjectDialog(int works, int contributions, const QString &serverName);
        ~TupListProjectDialog();
        void addWork(const QString &filename, const QString &name, const QString &description, const QString &date);
        void addContribution(const QString &filename, const QString &name, const QString &author, const QString &description, const QString &date);
        QString projectID() const;
        QString owner() const;
        bool workIsMine();
        
    private slots:
        void execAccept(QTreeWidgetItem *item, int index);
        void updateWorkTree();
        void updateContribTree();
    
    private:
        QTreeWidget *tree(bool myWorks);
        struct Private;
        Private *const k;
};

#endif
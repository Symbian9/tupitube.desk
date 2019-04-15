/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#ifndef TUPBRUSHMANAGER_H
#define TUPBRUSHMANAGER_H

#include "tglobal.h"

#include <QObject>
#include <QPen>
#include <QBrush>

class TUPITUBE_EXPORT TupBrushManager : public QObject
{
    Q_OBJECT

    public:
        TupBrushManager(QObject * parent = nullptr);
        TupBrushManager(const QPen &pen, const QBrush &brush, QObject * parent = nullptr);
        ~TupBrushManager();

        QPen pen() const;        
        void setPen(const QPen &pen);
        void setPenColor(const QColor &color);
        void setPenWidth(int width);

        QBrush brush() const; 
        void setBrush(const QBrush &brush);

        void initBgColor(const QColor &color);
        void setBgColor(const QColor &color);
        QColor bgColor();
        
        int penWidth() const;
        QColor penColor() const;
        QBrush penBrush() const;
        QBrush brushColor() const;
        
    signals:
        void penChanged(const QPen &pen);
        void brushChanged(const QBrush &brush);
        void bgColorChanged(const QColor color);
        
    private:
        QPen gPen;
        QBrush gBrush;
        QColor penBgColor;
};

#endif

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

#include "tuppaintareacommand.h"

struct TupPaintAreaCommand::Private
{
    TupPaintArea *paintArea;
    TupPaintAreaEvent *event;
    QVariant oldData;
};

TupPaintAreaCommand::TupPaintAreaCommand(TupPaintArea *area, const TupPaintAreaEvent *event) : QUndoCommand(), k(new Private)
{
    k->paintArea = area;
    k->event = event->clone();
}

TupPaintAreaCommand::~TupPaintAreaCommand()
{
    delete k;
}

void TupPaintAreaCommand::undo()
{
    switch(k->event->action()) {
           case TupPaintAreaEvent::ChangePen:
                {
                  k->paintArea->brushManager()->setPen(qvariant_cast<QPen>(k->oldData));
                }
                break;
           case TupPaintAreaEvent::ChangePenColor:
                {
                  k->paintArea->brushManager()->setPenColor(qvariant_cast<QColor>(k->oldData));
                }
                break;
           case TupPaintAreaEvent::ChangePenThickness:
                {
                  k->paintArea->brushManager()->setPenWidth(qvariant_cast<int>(k->oldData));
                }
                break;
           case TupPaintAreaEvent::ChangeBrush:
                {
                  k->paintArea->brushManager()->setBrush(qvariant_cast<QBrush>(k->oldData));
                }
                break;
           case TupPaintAreaEvent::ChangeBgColor:
                {
                  k->paintArea->brushManager()->setBgColor(qvariant_cast<QColor>(k->oldData));
                }
                break;
           default: 
                break;
    }
}

void TupPaintAreaCommand::redo()
{
    switch (k->event->action()) {
            case TupPaintAreaEvent::ChangePen:
                 {
                   k->oldData = k->paintArea->brushManager()->pen();
                   QPen pen = qvariant_cast<QPen>(k->event->data());
                   if (!pen.color().isValid()) {
                       QPen old = k->paintArea->brushManager()->pen();
                       pen.setColor(old.color());
                       pen.setBrush(old.brush());
                   } 
                   k->paintArea->brushManager()->setPen(pen);
                 }
                 break;
            case TupPaintAreaEvent::ChangePenColor:
                 {
                   k->oldData = k->paintArea->brushManager()->pen().color();
                   k->paintArea->brushManager()->setPenColor(qvariant_cast<QColor>(k->event->data()));
                 }
                 break;
            case TupPaintAreaEvent::ChangePenThickness:
                 {
                   k->oldData = k->paintArea->brushManager()->pen().width();
                   k->paintArea->brushManager()->setPenWidth(qvariant_cast<int>(k->event->data()));
                 }
                 break;
            case TupPaintAreaEvent::ChangeBrush:
                 {
                   k->oldData = k->paintArea->brushManager()->brush();
                   k->paintArea->brushManager()->setBrush(qvariant_cast<QBrush>(k->event->data()));
                 }
                 break;
            case TupPaintAreaEvent::ChangeBgColor:
                 {
                   k->oldData = k->paintArea->brushManager()->bgColor();
                   k->paintArea->brushManager()->setBgColor(qvariant_cast<QColor>(k->event->data()));
                 }
                 break;
            default: 
                 break;
    }
}

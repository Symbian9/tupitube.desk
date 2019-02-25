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

#include "polylinetool.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tuprequestbuilder.h"
#include "tuplibraryobject.h"
#include "taction.h"
#include "tupinputdeviceinformation.h"
#include "tupgraphicsscene.h"
#include "tupprojectrequest.h"
#include "tupbrushmanager.h"
#include "tupprojectresponse.h"

struct PolyLineTool::Private
{
    bool begin;
    QPointF center;
    QPointF right;
    QPointF mirror;
    QPointF lastPoint; 
    
    TNodeGroup *nodeGroup;
    QPainterPath path;
    
    QMap<QString, TAction *> actions;
    
    TupPathItem *item;
    TupGraphicsScene *scene;
    
    QGraphicsLineItem *line1;
    QGraphicsLineItem *line2;
    Settings *configurator;
    QCursor cursor;
    qreal realFactor;
    bool cutterOn;
    bool movingOn;
};

PolyLineTool::PolyLineTool(): k(new Private)
{
    k->configurator = nullptr;
    k->nodeGroup = nullptr;
    k->item = nullptr;
    k->cutterOn = false;
    k->movingOn = false;

    k->cursor = QCursor(kAppProp->themeDir() + "cursors/polyline.png", 4, 4);
    
    k->line1 = new QGraphicsLineItem(0, 0, 0, 0);
    k->line1->setPen(QPen(QColor(55, 177, 50)));
    k->line2 = new QGraphicsLineItem(0, 0, 0, 0);
    k->line2->setPen(QPen(QColor(55, 177, 50)));
    
    setupActions();
}

PolyLineTool::~PolyLineTool()
{
}

void PolyLineTool::setupActions()
{
    TAction *polyline = new TAction(QIcon(kAppProp->themeDir() + "/icons/polyline.png"), tr("PolyLine"), this);
    polyline->setShortcut(QKeySequence(tr("S")));
    polyline->setToolTip(tr("PolyLine") + " - " + "S");
    polyline->setCursor(k->cursor);

    k->actions.insert(tr("PolyLine"), polyline);
}

QMap<QString, TAction *> PolyLineTool::actions() const
{
    return k->actions;
}

void PolyLineTool::init(TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[PolyLineTool::init()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (scene) {
        k->scene = scene;
    } else {
        #ifdef TUP_DEBUG
            QString msg = "PolyLineTool::init() - Fatal Error: TupGraphicsScene variable is NULL!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return;
    }

    foreach (QGraphicsView *view, scene->views())
             view->setDragMode(QGraphicsView::NoDrag);

    k->cutterOn = false;
    initEnv();
}

QStringList PolyLineTool::keys() const
{
    return QStringList() << tr("PolyLine");
}

void PolyLineTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[PolyLineTool::press()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (k->cutterOn)
        return;

    // scene->clearSelection();

    if (k->begin) { // This condition happens only in the beginning of the polyline 
        k->path = QPainterPath();
        k->path.moveTo(input->pos());

        k->item = new TupPathItem();
        k->item->setPen(brushManager->pen());
        k->item->setBrush(brushManager->brush());
        k->item->setPath(k->path);
    } else { // This condition happens from the second point of the polyline and until the last one
        if (k->item) {
            if (!scene->items().contains(k->item))
                scene->includeObject(k->item, true); // SQA: Polyline hack

            if (k->movingOn) {
                k->path.cubicTo(k->right, k->mirror, input->pos());
                k->movingOn = false;
            } else {
                k->path.cubicTo(k->lastPoint, k->lastPoint, k->lastPoint);
            }
            k->item->setPath(k->path);
        }
    }
    
    k->center = input->pos();
    
    if (!scene->items().contains(k->line1))
        scene->includeObject(k->line1); 

    if (!scene->items().contains(k->line2))
        scene->includeObject(k->line2);
}

void PolyLineTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    Q_UNUSED(brushManager);
    Q_UNUSED(scene);

    if (k->cutterOn)
        return;

    k->movingOn = true;
    k->mirror = k->center - (input->pos() - k->center);
    if (k->begin) {
        k->right = input->pos();
    } else {
        for (int i=k->path.elementCount()-1; i>=0; i--) {
             if (k->path.elementAt(i).type == QPainterPath::CurveToElement) {
                 k->right = input->pos();
                 if (k->path.elementAt(i + 1).type == QPainterPath::CurveToDataElement)
                     k->path.setElementPositionAt(i + 1, k->mirror.x(), k->mirror.y());
                 break;
             }
        }
    }

    if (k->item)    
        k->item->setPath(k->path);
    if (k->line1)
        k->line1->setLine(QLineF(k->mirror, k->center));
    if (k->line2)
        k->line2->setLine(QLineF(k->right, k->center));
}

void PolyLineTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *scene)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[PolyLineTool::release()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    Q_UNUSED(input);
    Q_UNUSED(brushManager);

    if (k->cutterOn)
        return;

    if (k->begin && k->item) {
        QDomDocument doc;
        doc.appendChild(k->item->toXml(doc));
        TupProjectRequest request = TupRequestBuilder::createItemRequest(scene->currentSceneIndex(), 
                                                       scene->currentLayerIndex(), 
                                                       scene->currentFrameIndex(), 
                                                       0, QPointF(), scene->getSpaceContext(), TupLibraryObject::Item, 
                                                       TupProjectRequest::Add, doc.toString());
        emit requested(&request);
 
        k->begin = false;
    } else {
        if (k->item) {
            if (!k->nodeGroup) {
                k->nodeGroup = new TNodeGroup(k->item, k->scene, TNodeGroup::Polyline,
                                              static_cast<int>(k->item->zValue() + 1));
                connect(k->nodeGroup, SIGNAL(nodeReleased()), this, SLOT(nodeChanged()));
            } else {
                k->nodeGroup->createNodes(k->item);
            }

            k->nodeGroup->show();
            k->nodeGroup->resizeNodes(k->realFactor);
        }
    }
}

void PolyLineTool::itemResponse(const TupItemResponse *response)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[PolyLineTool::itemResponse()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    QGraphicsItem *item = nullptr;
    TupScene *scene = nullptr;
    TupLayer *layer = nullptr;
    TupFrame *frame = nullptr;

    TupProject *project = k->scene->currentScene()->project();
    scene = project->sceneAt(response->getSceneIndex());
    if (scene) {
        if (k->scene->getSpaceContext() == TupProject::FRAMES_EDITION) {
            layer = scene->layerAt(response->getLayerIndex());
            if (layer) {
                frame = layer->frameAt(response->getFrameIndex());
                if (frame) {
                    item = frame->item(response->getItemIndex());
                } else {
                    #ifdef TUP_DEBUG
                        QString msg = "PolyLineTool::itemResponse() - Fatal Error: Frame variable is NULL!";
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tError() << msg;
                        #endif
                    #endif
                }
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "PolyLineTool::itemResponse() - Fatal Error: Layer variable is NULL!";
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
            }
        } else {
            TupBackground *bg = scene->sceneBackground();
            if (bg) {
                if (k->scene->getSpaceContext() == TupProject::STATIC_BACKGROUND_EDITION) {
                    TupFrame *frame = bg->staticFrame();
                    if (frame) {
                        item = frame->item(response->getItemIndex());
                    } else {
                        #ifdef TUP_DEBUG
                           QString msg = "PolyLineTool::itemResponse() - Fatal Error: Static bg frame variable is NULL!";
                           #ifdef Q_OS_WIN
                               qDebug() << msg;
                           #else
                               tError() << msg;
                           #endif
                        #endif
                    }
                } else if (k->scene->getSpaceContext() == TupProject::DYNAMIC_BACKGROUND_EDITION) {
                           TupFrame *frame = bg->dynamicFrame();
                           if (frame) {
                               item = frame->item(response->getItemIndex());
                           } else {
                               #ifdef TUP_DEBUG
                                   QString msg = "PolyLineTool::itemResponse() - Fatal Error: Dynamic bg frame variable is NULL!";
                                   #ifdef Q_OS_WIN
                                       qDebug() << msg;
                                   #else
                                       tError() << msg;
                                   #endif
                               #endif
                           }
                } else {
                    #ifdef TUP_DEBUG
                        QString msg = "PolyLineTool::itemResponse() - Fatal Error: Invalid spaceContext!";
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tError() << msg;
                        #endif
                    #endif
                }
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "PolyLineTool::itemResponse() - Fatal Error: Scene bg variable is NULL!";
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
            }
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "PolyLineTool::itemResponse() - Fatal Error: Scene variable is NULL!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }

    switch (response->getAction()) {
        case TupProjectRequest::Add:
        {
            if (TupPathItem *path = qgraphicsitem_cast<TupPathItem *>(item)) {
                if (k->item != path) {
                    k->item = path;
                    if (k->nodeGroup)
                        k->nodeGroup->setParentItem(path);
                }
            }
        }
        break;
        case TupProjectRequest::Remove:
        {
            // SQA: Is this code doing something for real?
            /*
            if (item == k->item) {
                k->path = QPainterPath();
                delete k->item;
                k->item = 0;
                delete k->nodeGroup;
                k->nodeGroup = 0;
            }
            */
        }
        break;
        case TupProjectRequest::EditNodes:
        {
            if (item && k->nodeGroup) {
                if (qgraphicsitem_cast<QGraphicsPathItem *>(k->nodeGroup->parentItem()) == item) {
                    k->nodeGroup->show();
                    k->nodeGroup->syncNodesFromParent();
                    k->nodeGroup->saveParentProperties();

                    k->path = k->item->path();
                }
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "PolyLineTool::itemResponse() - Fatal Error: No item was found";
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
            }
        }
        break;
        default: 
        break;
    }
}

void PolyLineTool::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[PolyLineTool::keyPressEvent()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
    } else if (event->key() == Qt::Key_X) {
               k->cutterOn = true;
               initEnv();
    } else {
        QPair<int, int> flags = TupToolPlugin::setKeyAction(event->key(), event->modifiers());
        if (flags.first != -1 && flags.second != -1)
            emit callForPlugin(flags.first, flags.second);
    }
}

void PolyLineTool::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_X)
        k->cutterOn = false;
}

void PolyLineTool::initEnv()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[PolyLineTool::initEnv()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (k->item) {
        clearSelection();
        k->item = nullptr;
    }

    k->begin = true;
    k->movingOn = false;
    k->path = QPainterPath();

    if (k->line1) {
        if (k->scene->items().contains(k->line1))
            k->scene->removeItem(k->line1);
    }

    if (k->line2) {
        if (k->scene->items().contains(k->line2))
            k->scene->removeItem(k->line2);
    }
}

void PolyLineTool::nodeChanged()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[PolyLineTool::nodeChanged()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (k->nodeGroup) {
        if (!k->nodeGroup->changedNodes().isEmpty()) {
            int position = -1;
            if (k->scene->getSpaceContext() == TupProject::FRAMES_EDITION) {
                position = k->scene->currentFrame()->indexOf(k->nodeGroup->parentItem());
            } else {
                TupBackground *bg = k->scene->currentScene()->sceneBackground();
                if (bg) {
                    if (k->scene->getSpaceContext() == TupProject::STATIC_BACKGROUND_EDITION) {
                        TupFrame *frame = bg->staticFrame();
                        if (frame) {
                            position = frame->indexOf(k->nodeGroup->parentItem());
                        } else {
                            #ifdef TUP_DEBUG
                                QString msg = "PolyLineTool::nodeChanged() - Fatal Error: Static bg frame is NULL!";
                                #ifdef Q_OS_WIN
                                    qDebug() << msg;
                                #else
                                    tError() << msg;
                                #endif
                            #endif
                            return;
                        }
                    } else if (k->scene->getSpaceContext() == TupProject::DYNAMIC_BACKGROUND_EDITION) {
                               TupFrame *frame = bg->dynamicFrame();
                               if (frame) {
                                   position = frame->indexOf(k->nodeGroup->parentItem());
                               } else {
                                   #ifdef TUP_DEBUG
                                       QString msg = "PolyLineTool::nodeChanged() - Fatal Error: Dynamic bg frame is NULL!";
                                       #ifdef Q_OS_WIN
                                           qDebug() << msg;
                                       #else
                                           tError() << msg;
                                       #endif
                                   #endif
                                   return;
                               }
                    } else {
                        #ifdef TUP_DEBUG
                            QString msg = "PolyLineTool::nodeChanged() - Fatal Error: Invalid spaceContext!";
                            #ifdef Q_OS_WIN
                                qDebug() << msg;
                            #else
                                tError() << msg;
                            #endif
                        #endif
                    }
                } else {
                    #ifdef TUP_DEBUG
                        QString msg = "PolyLineTool::nodeChanged() - Fatal Error: Scene background variable is NULL!";
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tError() << msg;
                        #endif
                    #endif
                }
            }

            if (position >= 0) {
                TupPathItem *pathItem = qgraphicsitem_cast<TupPathItem *>(k->nodeGroup->parentItem());
                if (pathItem) {
                    QString path = pathItem->pathToString();
                    TupProjectRequest event = TupRequestBuilder::createItemRequest(k->scene->currentSceneIndex(), k->scene->currentLayerIndex(), k->scene->currentFrameIndex(), 
                                              position, QPointF(), k->scene->getSpaceContext(), TupLibraryObject::Item, TupProjectRequest::EditNodes, path);
                    emit requested(&event);
                    // k->nodeGroup->restoreItem();
                }
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "PolyLineTool::nodeChanged() - Fatal Error: Invalid object index || No nodeGroup parent item -> " + QString::number(position);
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                return;
             }
        } else {
          #ifdef TUP_DEBUG
              QString msg = "PolyLineTool::nodeChanged() - Fatal Error: Array of changed nodes is empty!";
              #ifdef Q_OS_WIN
                qDebug() << msg;
              #else
                tError() << msg;
              #endif
          #endif
          return;
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "PolyLineTool::nodeChanged() - Fatal Error: Array of nodes is empty!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }
}

int PolyLineTool::toolType() const
{
    return TupToolInterface::Brush;
}

QWidget *PolyLineTool::configurator() 
{
    if (! k->configurator)
        k->configurator = new Settings;

    return k->configurator;
}

void PolyLineTool::aboutToChangeScene(TupGraphicsScene *)
{
}

void PolyLineTool::aboutToChangeTool()
{
    k->nodeGroup = nullptr;
    k->item = nullptr;
}

void PolyLineTool::saveConfig()
{
}

QCursor PolyLineTool::cursor() const
{
    return k->cursor;
}

void PolyLineTool::resizeNodes(qreal scaleFactor)
{
    k->realFactor = scaleFactor;
    if (k->nodeGroup)
        k->nodeGroup->resizeNodes(scaleFactor);
}

void PolyLineTool::updateZoomFactor(qreal scaleFactor)
{
    k->realFactor = scaleFactor;
}

void PolyLineTool::clearSelection()
{
    if (k->nodeGroup) {
        k->nodeGroup->clear();
        k->nodeGroup = nullptr;
    }
}

void PolyLineTool::updatePos(QPointF pos)
{
    k->lastPoint = pos;
}

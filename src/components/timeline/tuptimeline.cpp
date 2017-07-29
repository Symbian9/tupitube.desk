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

#include "tuptimeline.h"
#include "tupprojectactionbar.h"
#include "tupscenecontainer.h"
#include "tupprojectrequest.h"

#include "tuplibraryobject.h"
#include "tuplayer.h"
#include "tuplibrary.h"

#include <QStackedWidget>
#include <QList>
#include <QHeaderView>

#define RETURN_IF_NOT_LIBRARY if (!k->library) return;

struct TupTimeLine::Private
{
    Private() : scenesContainer(0), actionBar(0), selectedLayer(-1), library(0) {}
    
    TupSceneContainer *scenesContainer;
    TupTimeLineTable *currentTable;
    TupProjectActionBar *actionBar;
    int selectedLayer; 
    TupProject *project;
    TupLibrary *library;
    QString frameSelection;
    bool doSelection;
};

TupTimeLine::TupTimeLine(TupProject *project, QWidget *parent) : TupModuleWidgetBase(parent, "TupTimeLine"), k(new Private)
{
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupTimeLine()]";
        #else
            TINIT;
        #endif
    #endif
    
    setWindowTitle(tr("Time Line"));
    setWindowIcon(QPixmap(THEME_DIR + "icons/time_line.png"));

    k->project = project;
    k->library = k->project->library();
    k->currentTable = 0;
    k->doSelection = false;

    // SQA: Pending to add the feature "Layer Opacity" as part of this action bar

    QList<TupProjectActionBar::Action> actions;
    actions << TupProjectActionBar::InsertFrame << TupProjectActionBar::ExtendFrame << TupProjectActionBar::RemoveFrame; 
    actions << TupProjectActionBar::MoveFrameBackward << TupProjectActionBar::MoveFrameForward;
    actions << TupProjectActionBar::CopyFrame << TupProjectActionBar::PasteFrame;
    actions << TupProjectActionBar::Separator;
    actions << TupProjectActionBar::InsertLayer << TupProjectActionBar::RemoveLayer;
    actions << TupProjectActionBar::Separator;
    actions << TupProjectActionBar::InsertScene << TupProjectActionBar::RemoveScene;

    k->actionBar = new TupProjectActionBar(QString("TimeLine"), actions);

    addChild(k->actionBar, Qt::AlignCenter);
    
    k->scenesContainer = new TupSceneContainer(this);
    addChild(k->scenesContainer);
    
    connect(k->actionBar, SIGNAL(actionSelected(int)), this, SLOT(requestCommand(int)));
    connect(k->scenesContainer, SIGNAL(currentChanged(int)), this, SLOT(requestSceneSelection(int)));
}

TupTimeLine::~TupTimeLine()
{
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupTimeLine()]";
        #else
            TEND;
        #endif
    #endif

    delete k;
}

TupTimeLineTable *TupTimeLine::framesTable(int sceneIndex)
{
    TupTimeLineTable *framesTable = qobject_cast<TupTimeLineTable *>(k->scenesContainer->widget(sceneIndex));
    if (framesTable)
        return framesTable;
 
    return 0;
}

void TupTimeLine::addScene(int sceneIndex, const QString &name)
{
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupTimeLine::addScene()]";
        #else
            T_FUNCINFO << "sceneIndex -> " << sceneIndex;
        #endif
    #endif

    if (sceneIndex < 0 || sceneIndex > k->scenesContainer->count()) {
        #ifdef K_DEBUG
            QString msg = "TupTimeLine::addScene() - Fatal error: invalid scene index -> " + QString::number(sceneIndex);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }

    TupTimeLineTable *framesTable = new TupTimeLineTable(sceneIndex, k->scenesContainer);
    framesTable->setItemSize(10, 20);

    connect(framesTable, SIGNAL(frameSelected(int, int)), this, SLOT(selectFrame(int, int)));
    connect(framesTable, SIGNAL(selectionCopied()), SLOT(requestCopyFrameSelection()));
    connect(framesTable, SIGNAL(selectionPasted()), SLOT(requestPasteSelectionInCurrentFrame()));
    connect(framesTable, SIGNAL(selectionRemoved()), SLOT(removeFrameSelection()));
    connect(framesTable, SIGNAL(frameExtended(int, int)), SLOT(extendFrameForward(int, int)));
    connect(framesTable, SIGNAL(visibilityChanged(int, bool)), this, SLOT(requestLayerVisibilityAction(int, bool)));

    connect(framesTable, SIGNAL(layerNameChanged(int, const QString &)), this, SLOT(requestLayerRenameAction(int, const QString &))); 
    connect(framesTable, SIGNAL(layerMoved(int, int)), this, SLOT(requestLayerMove(int, int)));
    connect(framesTable, SIGNAL(newPerspective(int)), this, SIGNAL(newPerspective(int)));

    k->scenesContainer->addScene(sceneIndex, framesTable, name);
}

void TupTimeLine::removeScene(int sceneIndex)
{
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupTimeLine::removeScene()]";
        #else
            T_FUNCINFO << "sceneIndex -> " << sceneIndex;
        #endif
    #endif

    if (sceneIndex >= 0 && sceneIndex < k->scenesContainer->count())
        k->scenesContainer->removeScene(sceneIndex, true);
}

void TupTimeLine::closeAllScenes()
{
    blockSignals(true);
    k->scenesContainer->removeAllScenes();
    blockSignals(false);
}

void TupTimeLine::sceneResponse(TupSceneResponse *response)
{
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupTimeLine::sceneResponse()]";
        #else
            T_FUNCINFO;
            T_FUNCINFO << "response->action() -> " << response->action();
        #endif
    #endif

    int sceneIndex = response->sceneIndex();

    switch (response->action()) {
        case TupProjectRequest::Add:
        {
            if (response->mode() == TupProjectResponse::Do) {
                addScene(sceneIndex, response->arg().toString());
                return;
            } else { 
                k->scenesContainer->restoreScene(sceneIndex, response->arg().toString());
                TupProjectRequest request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Select);
                emit requestTriggered(&request);
                return;
            }
        }
        break;
        case TupProjectRequest::Remove:
        {
            removeScene(sceneIndex);
        }
        break;
        case TupProjectRequest::Reset:
        {
            if (response->mode() == TupProjectResponse::Do || response->mode() == TupProjectResponse::Redo) {
                k->scenesContainer->removeScene(sceneIndex, true);
                addScene(sceneIndex, tr("Scene %1").arg(sceneIndex + 1));

                k->currentTable = k->scenesContainer->getTable(sceneIndex);
                k->currentTable->insertLayer(0, tr("Layer 1"));
                k->currentTable->insertFrame(0);

                k->currentTable->blockSignals(true);
                k->currentTable->selectFrame(0, 0);
                k->currentTable->blockSignals(false);
            } else {
                k->scenesContainer->removeScene(sceneIndex, false);
                k->scenesContainer->restoreScene(sceneIndex, response->arg().toString());

                k->currentTable = k->scenesContainer->getTable(sceneIndex);
                k->currentTable->blockSignals(true);
                k->currentTable->selectFrame(0, 0);
                k->currentTable->blockSignals(false);
            }
        }
        break;
        /*
        case TupProjectRequest::Move:
        {
            
        }
        break;
        case TupProjectRequest::Lock:
        {
            
        }
        break;
        */
        case TupProjectRequest::Rename:
        {
            k->scenesContainer->renameScene(sceneIndex, response->arg().toString());
        }
        break;
        case TupProjectRequest::Select:
        {
            k->scenesContainer->setCurrentIndex(sceneIndex);
        }
        break;
        default:
            #ifdef K_DEBUG
                QString msg = "TupTimeLine::sceneResponse : Unknown action -> " + QString::number(response->action());
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tFatal() << msg;
                #endif
            #endif
        break;
    }
}

void TupTimeLine::layerResponse(TupLayerResponse *response)
{
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupTimeLine::layerResponse()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    int sceneIndex = response->sceneIndex();
    TupTimeLineTable *framesTable = this->framesTable(sceneIndex);
    if (framesTable) {
        int layerIndex = response->layerIndex();
        switch (response->action()) {
            case TupProjectRequest::Add:
            {
                if (response->mode() == TupProjectResponse::Do) {
                    framesTable->insertLayer(layerIndex, response->arg().toString());
                    return;
                } else {
                    TupScene *scene = k->project->sceneAt(sceneIndex);
                    if (scene) {
                        TupLayer *layer = scene->layerAt(layerIndex);
                        if (layer) {
                            framesTable->insertLayer(layerIndex, layer->layerName());
                            QList<TupFrame *> frames = layer->frames();
                            int total = frames.count();
                            for (int i=0; i<total; i++)
                                framesTable->insertFrame(layerIndex);
                        }
                    }
                    return; 
                }
            }
            break;
            case TupProjectRequest::Remove:
            {
                framesTable->removeLayer(layerIndex);
                if (framesTable->layersCount() == 0) {
                    TupProjectRequest request = TupRequestBuilder::createLayerRequest(0, 0, TupProjectRequest::Add, tr("Layer %1").arg(1));
                    emit requestTriggered(&request);

                    request = TupRequestBuilder::createFrameRequest(0, 0, 0, TupProjectRequest::Add, tr("Frame"));
                    emit requestTriggered(&request);
                }
            }
            break;
            case TupProjectRequest::Move:
            {
                framesTable->moveLayer(layerIndex, response->arg().toInt());
            }
            break;
            /*
            case TupProjectRequest::Lock:
            {
                // SQA: Pending for implementation
            }
            */
            break;
            case TupProjectRequest::Rename:
            {
                framesTable->setLayerName(layerIndex, response->arg().toString());
            }
            break;
            case TupProjectRequest::View:
            {
                framesTable->setLayerVisibility(layerIndex, response->arg().toBool());
            }
            break;
        }
    }
}

void TupTimeLine::frameResponse(TupFrameResponse *response)
{
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupTimeLine::frameResponse()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    TupTimeLineTable *framesTable = this->framesTable(response->sceneIndex());
    if (framesTable) {
        int layerIndex = response->layerIndex();
        int frameIndex = response->frameIndex();

        switch (response->action()) {
            case TupProjectRequest::Add:
              {
                  framesTable->insertFrame(layerIndex);
              }
            break;
            case TupProjectRequest::Remove:
              {
                  framesTable->removeFrame(layerIndex, frameIndex);
              }
            break;
            case TupProjectRequest::RemoveSelection:
              {
                  QString selection = response->arg().toString();
                  if (response->mode() == TupProjectResponse::Do || response->mode() == TupProjectResponse::Redo) {
                      QStringList blocks = selection.split(":");
                      QStringList params = blocks.at(0).split(",");
                      int layers = params.at(0).toInt();
                      int frames = params.at(1).toInt();
                      framesTable->removeFrameSelection(layerIndex, frameIndex, layers, frames, k->doSelection);
                  } else {
                      framesTable->restoreFrameSelection(layerIndex, frameIndex, selection);
                  }

                  k->doSelection = false;
              }
            break;
            case TupProjectRequest::Exchange:
              {
                  framesTable->exchangeFrame(frameIndex, layerIndex, response->arg().toInt(), layerIndex);
              }
            break;
            case TupProjectRequest::Extend:
              {
                  int times = response->arg().toInt();
                  if (response->mode() == TupProjectResponse::Do || response->mode() == TupProjectResponse::Redo) {
                      for (int i=0; i<times; i++)
                          framesTable->insertFrame(layerIndex);
                  } else {
                      framesTable->removeFrameSelection(layerIndex, frameIndex, 1, times);
                  }
              }
            break;
            case TupProjectRequest::Select:
              {
                  QString selection = response->arg().toString();
                  k->selectedLayer = layerIndex;

                  framesTable->selectFrame(layerIndex, frameIndex, selection);
              }
            break;
            case TupProjectRequest::CopySelection:
              {
                  if (response->mode() == TupProjectResponse::Do)
                      k->frameSelection = response->arg().toString();
              }
            break;
            case TupProjectRequest::PasteSelection:
              {
                  if (!k->frameSelection.isEmpty()) {
                      QString selection = response->arg().toString();
                      QStringList params = selection.split(",");
                      if (params.count() == 4) {
                          QList<int> coords;
                          foreach(QString item, params)
                              coords << item.toInt();

                          int layers = coords.at(1) - coords.at(0) + 1;
                          int frames = coords.at(3) - coords.at(2) + 1;
                          if (response->mode() == TupProjectResponse::Do || response->mode() == TupProjectResponse::Redo)
                              framesTable->pasteFrameSelection(layerIndex, frameIndex, layers, frames);
                          else
                              framesTable->removeFrameSelection(layerIndex, frameIndex, layers, frames);
                      }
                  }
              }
            break;
            default:
              // Do nothing
            break;
        }
    }
}

void TupTimeLine::libraryResponse(TupLibraryResponse *response)
{
    // Q_UNUSED(response);

    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupTimeLine::libraryResponse()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (response->action() == TupProjectRequest::InsertSymbolIntoFrame) {
        switch (response->symbolType()) {
            case TupLibraryObject::Sound:
            {
                TupTimeLineTable *framesTable = this->framesTable(response->sceneIndex());
                if (framesTable) {
                    framesTable->insertSoundLayer(response->layerIndex() + 1, response->arg().toString());
                    framesTable->insertFrame(response->layerIndex() + 1);
                }
            }
            break;
            default:
                // Do nothing
            break;
        };
    }
}

void TupTimeLine::requestCommand(int action)
{
    int sceneIndex = k->scenesContainer->currentIndex();
    if (sceneIndex < 0) {
        #ifdef K_DEBUG
            QString msg = "TupTimeLine::requestCommand() - Fatal Error: Scene index is invalid -> " + QString::number(sceneIndex);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal("timeline") << msg;
            #endif
        #endif

        return;
    }

    int layerIndex = framesTable(sceneIndex)->currentLayer();
    if (layerIndex < 0) {
        #ifdef K_DEBUG
            QString msg = "TupTimeLine::requestCommand() - Fatal Error: Layer index is invalid -> " + QString::number(layerIndex);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal("timeline") << msg;
            #endif
        #endif

        return;
    }

    int frameIndex = framesTable(sceneIndex)->lastFrameByLayer(layerIndex);
    if (frameIndex < 0) {
        #ifdef K_DEBUG
            QString msg = "TupTimeLine::requestCommand() - Fatal Error: Frame index is invalid -> " + QString::number(frameIndex);
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal("timeline") << msg;
            #endif
        #endif

        return;
    }

    if (TupProjectActionBar::FrameActions & action) {
        if (!requestFrameAction(action, frameIndex, layerIndex, sceneIndex)) {
            #ifdef K_DEBUG
                QString msg = "TupTimeLine::requestCommand() - Fatal Error: Frame action has failed!";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tFatal("timeline") << msg;
                #endif
            #endif
        }
        return;
    }

    if (TupProjectActionBar::LayerActions & action) {
        if (!requestLayerAction(action, layerIndex, sceneIndex)) {
            #ifdef K_DEBUG
                QString msg = "TupTimeLine::requestCommand() - Fatal Error: Layer action has failed!";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tFatal("timeline") << msg;
                #endif
            #endif
        }
        return;
    }

    if (TupProjectActionBar::SceneActions & action) {
        if (!requestSceneAction(action, sceneIndex)) {
            #ifdef K_DEBUG
                QString msg = "TupTimeLine::requestCommand() - Fatal Error: Scene action has failed!";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tFatal("timeline") << msg;
                #endif
            #endif
        }
        return;
    }
}

bool TupTimeLine::requestFrameAction(int action, int frameIndex, int layerIndex, int sceneIndex, const QVariant &arg)
{
    /*
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupTimeLine::requestFrameAction()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
    */

    Q_UNUSED(frameIndex);
    Q_UNUSED(arg);

    TupProjectRequest request;
    int currentFrame = framesTable(sceneIndex)->currentColumn();

    switch (action) {
        case TupProjectActionBar::InsertFrame:
        {
            int lastFrame = framesTable(sceneIndex)->lastFrameByLayer(layerIndex);
            if (currentFrame == lastFrame) {
                request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, lastFrame + 1,
                                             TupProjectRequest::Add, tr("Frame"));
                emit requestTriggered(&request);
            } else {
                request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentFrame + 1,
                                             TupProjectRequest::Add, tr("Frame"));
                emit requestTriggered(&request);
                int target = currentFrame + 2;
                for (int index=target; index <= lastFrame+1; index++) {
                    target++;
                    request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, index, TupProjectRequest::Rename, tr("Frame"));
                    emit requestTriggered(&request);
                }
            }

            selectFrame(layerIndex, lastFrame + 1);
            return true;
        }
        break;
        case TupProjectActionBar::ExtendFrame:
        {
            extendFrameForward(layerIndex, currentFrame);
            return true;
        }
        break;
        case TupProjectActionBar::RemoveFrame:
        {
            requestRemoveFrame(true);
            return true;
        }
        break;
        case TupProjectActionBar::MoveFrameBackward:
        {
            TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentFrame, TupProjectRequest::Exchange, currentFrame - 1);
            emit requestTriggered(&request);

            return true;
        }
        break;
        case TupProjectActionBar::MoveFrameForward:
        {
            int lastFrame = framesTable(sceneIndex)->lastFrameByLayer(layerIndex);

            if (currentFrame == lastFrame) {
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, lastFrame + 1, TupProjectRequest::Add, tr("Frame"));
                emit requestTriggered(&request);
            }

            TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentFrame, TupProjectRequest::Exchange, currentFrame + 1);
            emit requestTriggered(&request);

            return true;
        }
        break;
        case TupProjectActionBar::CopyFrame:
        {
            requestCopyFrameSelection();

            return true;
        }
        break;
        case TupProjectActionBar::PasteFrame:
        {
            requestPasteSelectionInCurrentFrame();

            return true;
        }
        break;
        default:
            // Do nothing
        break;
    }
    
    return false;
}

bool TupTimeLine::requestLayerAction(int action, int layerIndex, int sceneIndex, const QVariant &arg)
{
    TupProjectRequest request;

    switch (action) {
        case TupProjectActionBar::InsertLayer:
        {
            int layerIndex = framesTable(sceneIndex)->layersCount();
            request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex, TupProjectRequest::Add, tr("Layer %1").arg(layerIndex + 1));
            emit requestTriggered(&request);

            if (layerIndex == 0) {
                request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, 0, TupProjectRequest::Add, tr("Frame"));
                emit requestTriggered(&request);
            } else {
                int total = framesTable(sceneIndex)->lastFrameByLayer(layerIndex - 1);
                for (int j=0; j <= total; j++) {
                    request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, j, TupProjectRequest::Add, tr("Frame"));
                    emit requestTriggered(&request);
                }
            }

            return true;
        }
        break;
        case TupProjectActionBar::RemoveLayer:
        {
            request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex, TupProjectRequest::Remove, arg);
            emit requestTriggered(&request);

            return true;
        }
        break;
    }
    
    return false;
}

bool TupTimeLine::requestSceneAction(int action, int sceneIndex, const QVariant &arg)
{
    TupProjectRequest request;

    switch (action) {
        case TupProjectActionBar::InsertScene:
        {
            int sceneTarget = k->scenesContainer->count();
            request = TupRequestBuilder::createSceneRequest(sceneTarget, TupProjectRequest::Add, tr("Scene %1").arg(sceneTarget + 1));
            emit requestTriggered(&request);

            request = TupRequestBuilder::createLayerRequest(sceneTarget, 0, TupProjectRequest::Add, tr("Layer 1"));
            emit requestTriggered(&request);

            request = TupRequestBuilder::createFrameRequest(sceneTarget, 0, 0, TupProjectRequest::Add, tr("Frame"));
            emit requestTriggered(&request);

            request = TupRequestBuilder::createSceneRequest(sceneTarget, TupProjectRequest::Select);
            emit requestTriggered(&request);
            
            return true;
        }
        break;
        case TupProjectActionBar::RemoveScene:
        {
            int scenesTotal = k->scenesContainer->count();
            if (scenesTotal > 1) {
                request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Remove, arg);
                emit requestTriggered(&request);

                request = TupRequestBuilder::createFrameRequest(sceneIndex - 1, 0, 0, TupProjectRequest::Select);
                emit requestTriggered(&request);
            } else {
                request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Reset, tr("Scene 1"));
                emit requestTriggered(&request);
            }

            return true;
        }
        break;
        case TupProjectActionBar::MoveSceneUp:
        {
            request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Move, sceneIndex + 1);
            emit requestTriggered(&request);

            return true;
        }
        break;
        case TupProjectActionBar::MoveSceneDown:
        {
            request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Move, sceneIndex - 1);
            emit requestTriggered(&request);

            return true;
        }
        break;
        default:
            // Do nothing
        break;
    }
    
    return false;
}

void TupTimeLine::requestLayerVisibilityAction(int layerIndex, bool isVisible)
{
    /*
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupTimeLine::requestLayerVisibilityAction()]";
        #else
            T_FUNCINFO;
        #endif
    #endif
    */

    int sceneIndex = k->scenesContainer->currentIndex();

    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex, TupProjectRequest::View, isVisible);
    emit requestTriggered(&request);
}

void TupTimeLine::requestLayerRenameAction(int layerIndex, const QString &name)
{
    /*
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupTimeLine::requestLayerRenameAction()]";
            qDebug() << "name: " << name;
        #else
            T_FUNCINFO << name;
        #endif
    #endif
    */

    int sceneIndex = k->scenesContainer->currentIndex();
    
    TupProjectRequest request = TupRequestBuilder::createLayerRequest(sceneIndex, layerIndex, TupProjectRequest::Rename, name);
    emit requestTriggered(&request);
}

void TupTimeLine::selectFrame(int layerIndex, int frameIndex)
{
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupTimeLine::selectFrame()]";
        #else
            T_FUNCINFO << "layerIndex, frameIndex -> (" << layerIndex << ", " << frameIndex << ")";
        #endif
    #endif

    int sceneIndex = k->scenesContainer->currentIndex();
    TupScene *scene = k->project->sceneAt(sceneIndex);
    if (scene) {
        int lastFrame = framesTable(sceneIndex)->lastFrameByLayer(layerIndex);
        QList<int> coords = framesTable(sceneIndex)->currentSelection();

        if (frameIndex > lastFrame) {
            for (int frame = lastFrame + 1; frame <= frameIndex; frame++) {
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frame,
                                            TupProjectRequest::Add, tr("Frame"));
                emit requestTriggered(&request);
            }
        }

        QString selection = "";
        if (coords.count() == 4) {
            if ((layerIndex >= coords.at(0) && layerIndex <= coords.at(1)) &&
                (frameIndex >= coords.at(2) && frameIndex <= coords.at(3))) {
                selection = QString::number(coords.at(0)) + "," + QString::number(coords.at(1)) + ","
                            + QString::number(coords.at(2)) + "," + QString::number(coords.at(3));
            } else {
                selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + ","
                            + QString::number(frameIndex) + "," + QString::number(frameIndex);
            }
        } else {
            selection = QString::number(layerIndex) + "," + QString::number(layerIndex) + ","
                        + QString::number(frameIndex) + "," + QString::number(frameIndex);
        }

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex,
                                                       frameIndex, TupProjectRequest::Select, selection);
        emit requestTriggered(&request);
    }
}

void TupTimeLine::removeFrameSelection()
{
    requestRemoveFrame(false);
}

void TupTimeLine::requestRemoveFrame(bool flag)
{
    k->doSelection = flag;

    int sceneIndex = k->scenesContainer->currentIndex();
    QList<int> coords = framesTable(sceneIndex)->currentSelection();
    if (coords.count() == 4) {
        int layers = coords.at(1) - coords.at(0) + 1;
        int frames = coords.at(3) - coords.at(2) + 1;

        QString flags = "";
        for (int i=coords.at(0); i<=coords.at(1); i++) {
            int framesCount = framesTable(sceneIndex)->lastFrameByLayer(i) + 1;
            if (framesCount > frames)
                flags += "0,";
            else
                flags += "1,";
        }
        flags.chop(1);

        QString selection = QString::number(layers) + "," + QString::number(frames) + ":" + flags;
        TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, coords.at(0), coords.at(2),
                                                                          TupProjectRequest::RemoveSelection, selection);
        emit requestTriggered(&request);
    }
}

void TupTimeLine::extendFrameForward(int layerIndex, int frameIndex)
{
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupCommandExecutor::copyFrameSelection()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    int sceneIndex = k->scenesContainer->currentIndex();
    TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, frameIndex,
                                                                      TupProjectRequest::Extend, 1);
    emit requestTriggered(&request);
}

void TupTimeLine::requestSceneSelection(int sceneIndex)
{
    if (k->scenesContainer->count() > 1) {
        TupProjectRequest request = TupRequestBuilder::createSceneRequest(sceneIndex, TupProjectRequest::Select);
        emit localRequestTriggered(&request);
    }
}

void TupTimeLine::requestLayerMove(int oldLayerIndex, int newLayerIndex)
{
    TupProjectRequest request = TupRequestBuilder::createLayerRequest(k->scenesContainer->currentIndex(), oldLayerIndex,
                                                   TupProjectRequest::Move, newLayerIndex);
    emit requestTriggered(&request);
}

void TupTimeLine::initLayerVisibility()
{
    #ifdef K_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "TupTimeLine::initLayerVisibility()";
        #else
            T_FUNCINFO << "TupTimeLine::initLayerVisibility()";
        #endif
    #endif

    int scenes = k->project->scenesCount();
    for (int sceneIndex=0; sceneIndex < scenes; sceneIndex++) {
         TupScene *scene = k->project->sceneAt(sceneIndex);
         if (scene) {
             int layers = scene->layersCount();
             for (int layerIndex=0; layerIndex < layers; layerIndex++) {
                  TupLayer *layer = scene->layerAt(layerIndex);
                  k->scenesContainer->getTable(sceneIndex)->setLayerVisibility(layerIndex, layer->isVisible());
             }
         }
    }
}

void TupTimeLine::requestCopyFrameSelection()
{
    int sceneIndex = k->scenesContainer->currentIndex();
    int layerIndex = framesTable(sceneIndex)->currentLayer();
    int currentFrame = framesTable(sceneIndex)->currentColumn();

    QList<int> coords = framesTable(sceneIndex)->currentSelection();
    if (coords.count() == 4) {
        QString selection = QString::number(coords.at(0)) + "," + QString::number(coords.at(1)) + ","
                            + QString::number(coords.at(2)) + "," + QString::number(coords.at(3));

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentFrame,
                                                                          TupProjectRequest::CopySelection, selection);
        emit requestTriggered(&request);
    }
}

void TupTimeLine::requestPasteSelectionInCurrentFrame()
{
    if (!k->frameSelection.isEmpty()) {
        int sceneIndex = k->scenesContainer->currentIndex();
        int layerIndex = framesTable(sceneIndex)->currentLayer();
        int currentFrame = framesTable(sceneIndex)->currentColumn();

        TupProjectRequest request = TupRequestBuilder::createFrameRequest(sceneIndex, layerIndex, currentFrame,
                                                                          TupProjectRequest::PasteSelection);
        emit requestTriggered(&request);
    }
}

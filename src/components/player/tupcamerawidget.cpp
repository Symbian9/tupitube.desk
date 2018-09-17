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

#include "tupcamerawidget.h"
#include "tconfig.h"
#include "tupinfodialog.h"

#include <QMainWindow>
#include <QLabel>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QProgressBar>

struct TupCameraWidget::Private
{
    QBoxLayout *layout;
    QFrame *container;
    TupScreen *screen;
    TupCameraBar *cameraBar;
    QProgressBar *progressBar;
    TupCameraStatus *status;
    TupProject *project;
    int currentSceneIndex;
    QLabel *scaleLabel;
    QSize playerDimension;
    QSize screenDimension;
    bool isScaled;
    QWidget *titleWidget;

    QLabel *currentFrameBox;
    QLabel *timerSecsLabel;
    QLabel *duration;
    int framesTotal;
    double fpsDelta;

    QLabel *projectName;
};

TupCameraWidget::TupCameraWidget(TupProject *project, bool isNetworked, QWidget *parent) : QFrame(parent), k(new Private)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupCameraWidget()]";
        #else
            TINIT;
        #endif
    #endif

    QDesktopWidget desktop;

    int percent = 40;
    int height = desktop.screenGeometry().height();
    if (height <= 700)
        percent = 30;
    if (height >= 1080)
        percent = 60;

    int desktopWidth = (percent * desktop.screenGeometry().width()) / 100;
    int desktopHeight = (percent * height) / 100;
    k->screenDimension = QSize(desktopWidth, desktopHeight);

    k->project = project;
    setObjectName("TupCameraWidget_");

    k->layout = new QBoxLayout(QBoxLayout::TopToBottom, this);

    addVideoHeader();
    addTimerPanel();
    k->layout->addSpacing(10);

    addAnimationDisplay();

    k->layout->addSpacing(10);
    addControlsBar();
    addStatusPanel(isNetworked);
}

TupCameraWidget::~TupCameraWidget()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupCameraWidget()]";
        #else
            TEND;
        #endif
    #endif

    if (k->cameraBar) {
        delete k->cameraBar;
        k->cameraBar = NULL;
    }

    if (k->progressBar) {
       delete k->progressBar;
       k->progressBar = NULL;
    }

    if (k->status) {
        delete k->status;
        k->status = NULL;
    }

    if (k->screen) {
        delete k->screen;
        k->screen = NULL;
    }

    delete k;
}

void TupCameraWidget::addVideoHeader()
{
    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->setAlignment(Qt::AlignCenter);
    labelLayout->setSpacing(0);

    QLabel *icon = new QLabel();
    icon->setPixmap(QPixmap(THEME_DIR + "icons/player.png"));
    QLabel *title = new QLabel(tr("Scene Preview"));
    QFont font = this->font();
    font.setBold(true);
    title->setFont(font);

    k->titleWidget = new QWidget();
    QHBoxLayout *titleLayout = new QHBoxLayout(k->titleWidget);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setAlignment(Qt::AlignCenter);
    titleLayout->addWidget(icon);
    titleLayout->addWidget(title);

    setProgressBar();

    k->projectName = new QLabel(k->project->projectName() + " ");
    k->projectName->setFont(font);

    setDimensionLabel(k->project->dimension());

    QWidget *scaleWidget = new QWidget();
    QHBoxLayout *scaleLayout = new QHBoxLayout(scaleWidget);
    scaleLayout->setContentsMargins(0, 0, 0, 0);
    scaleLayout->setAlignment(Qt::AlignCenter);
    scaleLayout->addWidget(k->scaleLabel);

    QPushButton *editButton = new QPushButton();
    editButton->setIcon(QIcon(THEME_DIR + "icons/edit_sign.png"));
    editButton->setFocusPolicy(Qt::NoFocus);
    editButton->setToolTip(tr("Edit Project Information"));
    connect(editButton, SIGNAL(pressed()), this, SLOT(infoDialog()));

    labelLayout->addWidget(k->projectName);
    labelLayout->addSpacing(5);
    labelLayout->addWidget(editButton);
    labelLayout->addSpacing(20);
    labelLayout->addWidget(scaleWidget);

    k->layout->addWidget(k->titleWidget, 0, Qt::AlignCenter);
    k->layout->addWidget(k->progressBar, 0, Qt::AlignCenter);
    k->layout->addLayout(labelLayout, Qt::AlignCenter);
}

void TupCameraWidget::setProgressBar()
{
    TCONFIG->beginGroup("General");
    QString themeName = TCONFIG->value("Theme", "Light").toString();

    k->progressBar = new QProgressBar(this);
    QString style1 = "QProgressBar { background-color: #DDDDDD; text-align: center; color: #FFFFFF; border-radius: 2px; } ";
    QString color = "#009500";
    if (themeName.compare("Dark") == 0)
        color = "#444444";
    QString style2 = "QProgressBar::chunk { background-color: " + color + "; border-radius: 2px; }";

    k->progressBar->setStyleSheet(style1 + style2);
    k->progressBar->setMaximumHeight(5);
    k->progressBar->setTextVisible(false);
    k->progressBar->setRange(1, 100);
    k->progressBar->setVisible(false);
}

void TupCameraWidget::addTimerPanel()
{
    QFont font = this->font();
    font.setBold(true);

    QLabel *timerFramesLabel = new QLabel(tr("Current Frame: "));
    timerFramesLabel->setFont(font);
    k->currentFrameBox = new QLabel("1");
    k->currentFrameBox->setAlignment(Qt::AlignCenter);
    k->currentFrameBox->setMinimumWidth(40);
    QString style = "QLabel { background-color: #c8c8c8; border: 1px solid #777777; border-radius: 2px; }";
    k->currentFrameBox->setStyleSheet(style);

    QLabel *stopwatchLabel = new QLabel(tr("Timer: "));
    stopwatchLabel->setFont(font);
    k->timerSecsLabel = new QLabel("00.00");
    k->timerSecsLabel->setAlignment(Qt::AlignCenter);
    k->timerSecsLabel->setMinimumWidth(50);
    k->timerSecsLabel->setStyleSheet(style);

    QLabel *durationLabel = new QLabel(tr("Duration: "));
    durationLabel->setFont(font);
    k->duration = new QLabel("");

    QFrame *timerWidget = new QFrame(this);
    timerWidget->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QHBoxLayout *timerLayout = new QHBoxLayout(timerWidget);
    timerLayout->setContentsMargins(10, 5, 10, 5);
    timerLayout->setAlignment(Qt::AlignCenter);
    timerLayout->addWidget(timerFramesLabel);
    timerLayout->addWidget(k->currentFrameBox);
    timerLayout->addSpacing(10);

    timerLayout->addWidget(stopwatchLabel);
    timerLayout->addWidget(k->timerSecsLabel);
    timerLayout->addWidget(new QLabel(tr("secs")));
    timerLayout->addSpacing(10);

    timerLayout->addWidget(durationLabel);
    timerLayout->addWidget(k->duration);

    k->layout->addWidget(timerWidget, 0, Qt::AlignCenter|Qt::AlignTop);
}

void TupCameraWidget::addAnimationDisplay()
{
    k->screen = new TupScreen(k->project, k->playerDimension, k->isScaled);
    connect(k->screen, SIGNAL(isRendering(int)), this, SLOT(updateProgressBar(int)));
    connect(k->screen, SIGNAL(frameChanged(int)), this, SLOT(updateTimerPanel(int)));

    k->layout->addWidget(k->screen, 0, Qt::AlignCenter);
}

void TupCameraWidget::addControlsBar()
{
    k->cameraBar = new TupCameraBar;

    connect(k->cameraBar, SIGNAL(play()), this, SLOT(doPlay()));
    connect(k->cameraBar, SIGNAL(playBack()), this, SLOT(doPlayBack()));
    connect(k->cameraBar, SIGNAL(pause()), k->screen, SLOT(pause()));
    connect(k->cameraBar, SIGNAL(stop()), k->screen, SLOT(stop()));
    connect(k->cameraBar, SIGNAL(ff()), k->screen, SLOT(nextFrame()));
    connect(k->cameraBar, SIGNAL(rew()), k->screen, SLOT(previousFrame()));

    k->layout->addWidget(k->cameraBar, 0, Qt::AlignCenter);
}

void TupCameraWidget::addStatusPanel(bool isNetworked)
{
    k->status = new TupCameraStatus(this, isNetworked);
    k->status->setScenes(k->project);
    connect(k->status, SIGNAL(sceneIndexChanged(int)), this, SLOT(selectScene(int)));
    connect(k->status, SIGNAL(muteEnabled(bool)), k->screen, SLOT(enableMute(bool)));
    connect(k->status, SIGNAL(fpsChanged(int)), this,  SLOT(setDuration(int)));

    updateFramesTotal(0);
    int fps = k->project->fps();
    k->fpsDelta = 1.0/fps;
    k->status->setFPS(fps);
    setLoop();
    k->layout->addWidget(k->status, 0, Qt::AlignCenter|Qt::AlignTop);
}

void TupCameraWidget::setDimensionLabel(const QSize dimension)
{
    QFont font = this->font();
    font.setBold(false);
    k->scaleLabel = new QLabel;
    k->scaleLabel->setFont(font);

    int screenWidth = k->screenDimension.width();
    int screenHeight = k->screenDimension.height();

    int projectWidth = dimension.width();
    int projectHeight = dimension.height();

    QString scale = "<b>[</b> " + tr("Scale") + " ";
    k->isScaled = false;

    if (projectWidth <= screenWidth && projectHeight <= screenHeight) {
        k->playerDimension = k->project->dimension();
        scale += "1:1";
    } else {
        double proportion = 1;

        if (projectWidth > projectHeight) {
            int newH = (projectHeight*screenWidth)/projectWidth;
            k->playerDimension = QSize(screenWidth, newH);
            proportion = (double) projectWidth / (double) screenWidth;
        } else { // projectHeight > projectWidth
            int newW = (projectWidth*screenHeight)/projectHeight;
            k->playerDimension = QSize(newW, screenHeight);
            proportion = (double) projectHeight / (double) screenHeight;
        }

        scale += "1:" + QString::number(proportion, 'g', 2);
        k->isScaled = true;
    }

    scale += " | " + tr("Size") + ": ";
    scale += QString::number(projectWidth) + "x" + QString::number(projectHeight);
    scale += " px <b>]</b>";

    k->scaleLabel->setText(scale);
}

void TupCameraWidget::setLoop()
{
    k->screen->setLoop(k->status->isLooping());
}

QSize TupCameraWidget::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    return size.expandedTo(QApplication::globalStrut());
}

void TupCameraWidget::doPlay()
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[TupCameraWidget::doPlay()]";
    #else
        T_FUNCINFO;
    #endif
#endif

    k->screen->play();
    bool flag = false;
    if (k->screen->currentSceneFrames() > 1)
        flag = true;
    k->status->enableExportButton(flag);
}

void TupCameraWidget::doPlayBack()
{
    k->screen->playBack();
}

void TupCameraWidget::doPause()
{
    k->screen->pause();
}

void TupCameraWidget::doStop()
{
    k->screen->stop();
}

void TupCameraWidget::nextFrame()
{
    k->screen->nextFrame();
}

void TupCameraWidget::previousFrame()
{
    k->screen->previousFrame();
}

bool TupCameraWidget::handleProjectResponse(TupProjectResponse *response)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupCameraWidget::handleProjectResponse()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (TupSceneResponse *sceneResponse = static_cast<TupSceneResponse *>(response)) {
        int index = sceneResponse->sceneIndex();

        switch (sceneResponse->action()) {
            case TupProjectRequest::Add:
            {
                 k->status->setScenes(k->project);
                 k->status->setCurrentScene(index);
                 updateFramesTotal(index);
            }
            break;
            case TupProjectRequest::Remove:
            {
                 if (index < 0)
                     break;

                 if (index == k->project->scenesCount())
                     index--;

                 if (index >= 0) {
                     k->status->setScenes(k->project);
                     k->status->setCurrentScene(index);
                     updateFramesTotal(index);
                 }
            }
            break;
            case TupProjectRequest::Reset:
            {
                 k->status->setScenes(k->project);
            }
            break;
            case TupProjectRequest::Select:
            {
                 if (index >= 0) {
                     k->currentSceneIndex = index;
                     updateFramesTotal(index);
                     k->status->setCurrentScene(index);
                 }
            }
            break;
            case TupProjectRequest::Rename:
            {
                 k->status->setScenes(k->project);
                 k->status->setCurrentScene(index);
            }
            break;
            default:
            {
                 #ifdef TUP_DEBUG
                     QString msg = "TupCameraWidget::handleProjectResponse() - Unknown/Unhandled project action: " 
                                   + QString::number(sceneResponse->action());
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tFatal() << msg;
                     #endif
                 #endif
            }
            break;
        }
    }

    return k->screen->handleResponse(response);
}

void TupCameraWidget::setFPS(int fps)
{
    k->project->setFPS(fps);
    k->screen->setFPS(fps);
    k->fpsDelta = 1.0/fps;
}

void TupCameraWidget::setStatusFPS(int fps)
{
    k->status->blockSignals(true);
    k->status->setFPS(fps);
    k->status->blockSignals(false);

    k->project->setFPS(fps);
    k->screen->setFPS(fps);
    setDuration(fps);
}

void TupCameraWidget::updateFramesTotal(int sceneIndex)
{
    TupScene *scene = k->project->sceneAt(sceneIndex);
    if (scene) {
        k->framesTotal = scene->framesCount();
        k->status->setFramesTotal(QString::number(k->framesTotal)); 
        k->progressBar->setRange(0, k->framesTotal);
        setDuration(k->project->fps());
    }
}

void TupCameraWidget::exportDialog()
{
    QDesktopWidget desktop;

    TupExportWidget *exportWidget = new TupExportWidget(k->project, this);
    exportWidget->show();
    exportWidget->move((int) (desktop.screenGeometry().width() - exportWidget->width())/2, 
                       (int) (desktop.screenGeometry().height() - exportWidget->height())/2);
    exportWidget->exec();
}

void TupCameraWidget::postDialog()
{
    QDesktopWidget desktop;

    TupExportWidget *exportWidget = new TupExportWidget(k->project, this, false);
    exportWidget->show();
    exportWidget->move((int) (desktop.screenGeometry().width() - exportWidget->width())/2,
                       (int) (desktop.screenGeometry().height() - exportWidget->height())/2);
    exportWidget->exec();

    if (exportWidget->isComplete() != QDialog::Rejected) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        emit requestForExportVideoToServer(exportWidget->videoTitle(), exportWidget->videoTopics(), 
                                           exportWidget->videoDescription(), k->status->getFPS(), 
                                           exportWidget->videoScenes());
    }
}

void TupCameraWidget::selectScene(int index)
{
    if (index != k->screen->currentSceneIndex()) {
        TupProjectRequest event = TupRequestBuilder::createSceneRequest(index, TupProjectRequest::Select);
        emit requestTriggered(&event);

        doStop();
        k->screen->updateSceneIndex(index);
        k->screen->updateAnimationArea();
        doPlay();
    }
}

void TupCameraWidget::updateScenes(int sceneIndex)
{
    k->screen->resetPhotograms(sceneIndex);
}

void TupCameraWidget::updateFirstFrame()
{
    k->screen->updateAnimationArea();
    k->currentFrameBox->setText("1");
}

void TupCameraWidget::updateProgressBar(int advance)
{
    if (advance == 0) {
        k->titleWidget->setVisible(true);
        k->progressBar->setVisible(false);
    }

    if (advance == 1) {
        k->titleWidget->setVisible(false);
        k->progressBar->setVisible(true);
    }

    k->progressBar->setValue(advance);
}

void TupCameraWidget::updateTimerPanel(int currentFrame)
{
    k->currentFrameBox->setText(QString::number(currentFrame));
    double time = k->fpsDelta * currentFrame;
    k->timerSecsLabel->setText(QString::number(time, 'f', 2));
}

void TupCameraWidget::updateSoundItems()
{
    k->screen->loadSoundRecords();
}

void TupCameraWidget::setDuration(int fps)
{
    qreal duration = (qreal) k->framesTotal / (qreal) fps;
    k->duration->setText(QString::number(duration, 'f', 2) + QString(" " + tr("secs")));
}

void TupCameraWidget::infoDialog()
{
    TupInfoDialog *settings = new TupInfoDialog(k->project->tags(), 
                                                        k->project->author(), k->project->description(), 
                                                        this);
    connect(settings, SIGNAL(dataSent(const QString &, const QString &, const QString &)), 
            this, SLOT(saveProjectInfo(const QString &, const QString &, const QString &)));

    settings->show();
}

void TupCameraWidget::saveProjectInfo(const QString &tags, const QString &author, 
                                      const QString &description) 
{
    k->project->setTags(tags);
    k->project->setAuthor(author);
    emit projectAuthorUpdated(author);
    k->project->setDescription(description);
}

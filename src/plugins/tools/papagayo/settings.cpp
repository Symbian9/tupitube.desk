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

#include "settings.h"
#include "tuplipsync.h"
#include "timagebutton.h"

struct Settings::Private
{
    QWidget *innerPanel;
    QBoxLayout *layout;

    QLabel *lipSyncName;
    QLabel *fpsLabel;
    QSpinBox *comboInit;

    QLabel *endingLabel;
    QLabel *totalLabel;

    QListWidget *mouthsList;
    QList<TupVoice *> voices;

    QTextEdit *textArea;

    // QString phoneme;
    QLabel *phonemeLabel;
    QSpinBox *xPosField;
    QSpinBox *yPosField;

    QString name;
    int initFrame;
    int framesCount;
};

Settings::Settings(QWidget *parent) : QWidget(parent), k(new Private)
{
    k->layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    k->layout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    setInnerForm();
}

Settings::~Settings()
{
    delete k;
}

void Settings::setInnerForm()
{
    k->innerPanel = new QWidget;

    QBoxLayout *innerLayout = new QBoxLayout(QBoxLayout::TopToBottom, k->innerPanel);
    innerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QLabel *nameLabel = new QLabel(tr("Editing") + ": ");
    k->lipSyncName = new QLabel;

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    nameLayout->setMargin(0);
    nameLayout->setSpacing(0);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(k->lipSyncName);

    k->fpsLabel = new QLabel;

    QHBoxLayout *fpsLayout = new QHBoxLayout;
    fpsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    fpsLayout->setMargin(0);
    fpsLayout->setSpacing(0);
    fpsLayout->addWidget(k->fpsLabel);

    QLabel *startingLabel = new QLabel(tr("Starting at frame") + ": ");
    startingLabel->setAlignment(Qt::AlignVCenter);

    k->comboInit = new QSpinBox();
    k->comboInit->setEnabled(false);
    k->comboInit->setMinimum(1);
    k->comboInit->setMaximum(999);
    connect(k->comboInit, SIGNAL(valueChanged(int)), this, SLOT(updateInitFrame(int)));
 
    k->endingLabel = new QLabel;
    k->endingLabel->setAlignment(Qt::AlignVCenter);

    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->setAlignment(Qt::AlignHCenter);
    startLayout->setMargin(0);
    startLayout->setSpacing(0);
    startLayout->addWidget(startingLabel);
    startLayout->addWidget(k->comboInit);

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->setAlignment(Qt::AlignHCenter);
    endLayout->setMargin(0);
    endLayout->setSpacing(0);
    endLayout->addWidget(k->endingLabel);

    k->totalLabel = new QLabel;
    k->totalLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    QHBoxLayout *totalLayout = new QHBoxLayout;
    totalLayout->setAlignment(Qt::AlignHCenter);
    totalLayout->setMargin(0);
    totalLayout->setSpacing(0);
    totalLayout->addWidget(k->totalLabel);

    QBoxLayout *listLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    listLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QLabel *mouthsLabel = new QLabel(tr("Mouths") + ": ");
    mouthsLabel->setAlignment(Qt::AlignHCenter);

    k->mouthsList = new QListWidget;
    k->mouthsList->setContextMenuPolicy(Qt::CustomContextMenu);
    k->mouthsList->setViewMode(QListView::ListMode);
    k->mouthsList->setFlow(QListView::TopToBottom);
    k->mouthsList->setMovement(QListView::Static);
    k->mouthsList->setFixedHeight(68);

    listLayout->addWidget(mouthsLabel);
    listLayout->addWidget(k->mouthsList);

    QLabel *textLabel = new QLabel(tr("Text") + ": ");
    textLabel->setAlignment(Qt::AlignHCenter);

    k->textArea = new QTextEdit;
    k->textArea->setReadOnly(true);

    // k->phonemeLabel = new QLabel(tr("Current Phoneme") + ": " + k->phoneme);
    k->phonemeLabel = new QLabel;
    k->phonemeLabel->setAlignment(Qt::AlignHCenter);

    QLabel *mouthPosLabel = new QLabel(tr("Current Mouth Position") + ": ");
    mouthPosLabel->setAlignment(Qt::AlignHCenter);

    QLabel *xLabel = new QLabel(tr("X") + ": ");
    xLabel->setMaximumWidth(20);

    k->xPosField = new QSpinBox;
    k->xPosField->setMinimum(-5000);
    k->xPosField->setMaximum(5000);
    connect(k->xPosField, SIGNAL(valueChanged(int)), this, SIGNAL(xPosChanged(int)));

    QLabel *yLabel = new QLabel(tr("Y") + ": ");
    yLabel->setMaximumWidth(20);

    k->yPosField = new QSpinBox;
    k->yPosField->setMinimum(-5000);
    k->yPosField->setMaximum(5000);
    connect(k->yPosField, SIGNAL(valueChanged(int)), this, SIGNAL(yPosChanged(int)));

    QBoxLayout *xLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    xLayout->setMargin(0);
    xLayout->setSpacing(0);
    xLayout->addWidget(xLabel);
    xLayout->addWidget(k->xPosField);

    QBoxLayout *yLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    yLayout->setMargin(0);
    yLayout->setSpacing(0);
    yLayout->addWidget(yLabel);
    yLayout->addWidget(k->yPosField);

    TImageButton *remove = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/close_properties.png"), 22);
    remove->setToolTip(tr("Close properties"));
    connect(remove, SIGNAL(clicked()), this, SIGNAL(closeLipSyncProperties()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(remove);

    innerLayout->addLayout(nameLayout);
    innerLayout->addLayout(fpsLayout);
    innerLayout->addLayout(startLayout);
    innerLayout->addLayout(endLayout);
    innerLayout->addLayout(totalLayout);
    innerLayout->addLayout(listLayout);
    innerLayout->addWidget(textLabel);
    innerLayout->addWidget(k->textArea);
    innerLayout->addWidget(k->phonemeLabel);
    innerLayout->addWidget(mouthPosLabel);
    innerLayout->addLayout(xLayout);
    innerLayout->addLayout(yLayout);

    innerLayout->addSpacing(10);
    innerLayout->addLayout(buttonsLayout);
    innerLayout->addSpacing(5);

    k->layout->addWidget(k->innerPanel);
}

// Editing new LipSync 

void Settings::openLipSyncProperties(TupLipSync *lipsync)
{
    k->name = lipsync->getLipSyncName();
    k->initFrame = lipsync->getInitFrame();
    k->framesCount = lipsync->getFramesCount();

    k->lipSyncName->setText(k->name);
    k->fpsLabel->setText(tr("Lip-Sync FPS") + ": " + QString::number(lipsync->getFPS()));

    k->comboInit->setEnabled(true);
    k->comboInit->setValue(k->initFrame + 1);

    int endIndex = k->initFrame + k->framesCount;
    k->endingLabel->setText(tr("Ending at frame") + ": " + QString::number(endIndex));
    k->totalLabel->setText(tr("Frames Total") + ": " + QString::number(k->framesCount));

    disconnect(k->mouthsList, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentMouth(int)));
    k->mouthsList->clear();

    k->voices = lipsync->getVoices();
    int total = k->voices.size();
    if (total > 0) {
        for (int i=0; i < total; i++) {
             QListWidgetItem *item = new QListWidgetItem(k->mouthsList);
             item->setText(tr("mouth") + "_" + QString::number(i));
             item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        }

        TupVoice *voice = k->voices.at(0);
        k->textArea->setText(voice->text());

        connect(k->mouthsList, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentMouth(int)));
        k->mouthsList->setCurrentRow(0);
    }
}

void Settings::setCurrentMouth(int index) 
{
    QString tail = ":" + QString::number(index);
    QString id = "lipsync:" + k->name + tail;

    TupVoice *voice = k->voices.at(index);
    k->textArea->setText(voice->text());

    emit selectMouth(id, index);
}

void Settings::updateInitFrame(int index)
{
    int frame = index - 1;

    if (frame != k->initFrame) {
        k->initFrame = frame;
        emit initFrameHasChanged(frame);
    }
}

void Settings::updateInterfaceRecords()
{
    int endIndex = k->initFrame + k->framesCount;
    k->endingLabel->setText(tr("Ending at frame") + ": " + QString::number(endIndex));
}

void Settings::setPos(const QPointF &point) 
{
    qreal x = point.x();
    qreal y = point.y();

    k->xPosField->blockSignals(true);
    k->yPosField->blockSignals(true);

    k->xPosField->setValue(x);
    k->yPosField->setValue(y);

    k->xPosField->blockSignals(false);
    k->yPosField->blockSignals(false);
}

void Settings::setPhoneme(const QString &phoneme)
{
    k->phonemeLabel->setText(tr("Current Phoneme") + ": " + "<b>" + phoneme + "</b>");
}

/***************************************************************************
 *   Project TUPITUBE DESK                           *
 *   Project Contact: info@maefloresta.com                   *
 *   Project Website: http://www.maefloresta.com                    *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>         *
 *                                             *
 *   Developers:                                      *
 *   2010:                                     *
 *    Gustavo Gonzalez / xtingray                            *
 *                                             *
 *   KTooN's versions:                                * 
 *                                             *
 *   2006:                                     *
 *    David Cuadrado                                  *
 *    Jorge Cuadrado                                  *
 *   2003:                                     *
 *    Fernado Roldan                                  *
 *    Simena Dinas                                    *
 *                                             *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                  *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                     *
 *                                             *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                   *
 *                                             *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "settings.h"
#include "tradiobuttongroup.h"
#include "tupitemtweener.h"
#include "tuptweenerstep.h"
#include "timagebutton.h"
#include "tseparator.h"
#include "tosd.h"

#include <math.h>

struct Settings::Private
{
    QWidget *innerPanel;
    QWidget *rangePanel;

    QBoxLayout *layout;
    TupToolPlugin::Mode mode;

    QLineEdit *input;
    TRadioButtonGroup *options;
    QSpinBox *initFrame;
    QSpinBox *endFrame;

    QComboBox *rotationTypeCombo;
    TupItemTweener::RotationType rotationType;

    QSpinBox *rangeStart;
    QSpinBox *rangeEnd;

    QDoubleSpinBox *degreesPerFrame;

    QCheckBox *rangeLoopBox;
    QCheckBox *reverseLoopBox;
    QLabel *totalLabel;
    QComboBox *clockCombo;
    int totalSteps;

    bool selectionDone;
    bool propertiesDone;

    TImageButton *apply;
    TImageButton *remove;
};

Settings::Settings(QWidget *parent) : QWidget(parent), k(new Private)
{
    k->selectionDone = false;
    k->propertiesDone = false;
    k->rotationType = TupItemTweener::Continuos;
    k->totalSteps = 0;

    k->layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    k->layout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QLabel *nameLabel = new QLabel(tr("Name") + ": ");
    k->input = new QLineEdit;

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    nameLayout->setMargin(0);
    nameLayout->setSpacing(0);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(k->input);

    k->options = new TRadioButtonGroup(tr("Options"), Qt::Vertical);
    k->options->addItem(tr("Select object"), 0);
    k->options->addItem(tr("Set Properties"), 1);
    connect(k->options, SIGNAL(clicked(int)), this, SLOT(emitOptionChanged(int)));

    k->apply = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/save.png"), 22);
    connect(k->apply, SIGNAL(clicked()), this, SLOT(applyTween()));

    k->remove = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/close.png"), 22);
    connect(k->remove, SIGNAL(clicked()), this, SIGNAL(clickedResetTween()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(k->apply);
    buttonsLayout->addWidget(k->remove);

    k->layout->addLayout(nameLayout);
    k->layout->addWidget(k->options);

    setInnerForm();

    k->layout->addSpacing(10);
    k->layout->addLayout(buttonsLayout);
    k->layout->setSpacing(5);

    activateMode(TupToolPlugin::Selection);
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

    QLabel *startingLabel = new QLabel(tr("Starting at frame") + ": ");
    startingLabel->setAlignment(Qt::AlignVCenter);

    k->initFrame = new QSpinBox();
    k->initFrame->setEnabled(false);
    k->initFrame->setMaximum(999);
    connect(k->initFrame, SIGNAL(valueChanged(int)), this, SLOT(updateRangeFromInit(int)));
    // connect(k->initFrame, SIGNAL(valueChanged(int)), this, SLOT(updateLastFrame()));
 
    QLabel *endingLabel = new QLabel(tr("Ending at frame") + ": ");
    endingLabel->setAlignment(Qt::AlignVCenter);

    k->endFrame = new QSpinBox();
    k->endFrame->setEnabled(true);
    k->endFrame->setValue(1);
    k->endFrame->setMaximum(999);
    connect(k->endFrame, SIGNAL(valueChanged(int)), this, SLOT(updateRangeFromEnd(int)));
    // connect(k->endFrame, SIGNAL(valueChanged(int)), this, SLOT(checkTopLimit(int)));

    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->setAlignment(Qt::AlignHCenter);
    startLayout->setMargin(0);
    startLayout->setSpacing(0);
    startLayout->addWidget(startingLabel);
    startLayout->addWidget(k->initFrame);

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->setAlignment(Qt::AlignHCenter);
    endLayout->setMargin(0);
    endLayout->setSpacing(0);
    endLayout->addWidget(endingLabel);
    endLayout->addWidget(k->endFrame);

    k->totalLabel = new QLabel(tr("Frames Total") + ": 1");
    k->totalLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    QHBoxLayout *totalLayout = new QHBoxLayout;
    totalLayout->setAlignment(Qt::AlignHCenter);
    totalLayout->setMargin(0);
    totalLayout->setSpacing(0);
    totalLayout->addWidget(k->totalLabel);

    k->rotationTypeCombo = new QComboBox();
    k->rotationTypeCombo->addItem(tr("Continuous"));
    k->rotationTypeCombo->addItem(tr("Partial"));

    connect(k->rotationTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(refreshForm(int)));

    QLabel *typeLabel = new QLabel(tr("Type") + ": ");
    typeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *typeLayout = new QHBoxLayout;
    typeLayout->setAlignment(Qt::AlignHCenter);
    typeLayout->setMargin(0);
    typeLayout->setSpacing(0);
    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(k->rotationTypeCombo);

    QLabel *speedLabel = new QLabel(tr("Speed (Degrees/Frame)") + ": ");
    speedLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    k->degreesPerFrame = new QDoubleSpinBox;
    k->degreesPerFrame->setEnabled(true);
    k->degreesPerFrame->setDecimals(2);
    k->degreesPerFrame->setMinimum(0.01);
    k->degreesPerFrame->setMaximum(360);
    k->degreesPerFrame->setSingleStep(0.05);
    k->degreesPerFrame->setValue(1);

    QVBoxLayout *speedLayout = new QVBoxLayout;
    speedLayout->setAlignment(Qt::AlignHCenter);
    speedLayout->setMargin(0);
    speedLayout->setSpacing(0);
    speedLayout->addWidget(speedLabel);

    QVBoxLayout *speedLayout2 = new QVBoxLayout;
    speedLayout2->setAlignment(Qt::AlignHCenter);
    speedLayout2->setMargin(0);
    speedLayout2->setSpacing(0);
    speedLayout2->addWidget(k->degreesPerFrame);

    innerLayout->addLayout(startLayout);
    innerLayout->addLayout(endLayout);
    innerLayout->addLayout(totalLayout);

    innerLayout->addSpacing(15);
    innerLayout->addWidget(new TSeparator(Qt::Horizontal));
    innerLayout->addLayout(typeLayout);

    innerLayout->addWidget(new TSeparator(Qt::Horizontal));

    QBoxLayout *clockLayout = new QBoxLayout(QBoxLayout::TopToBottom); // , k->clockPanel);
    clockLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    clockLayout->setMargin(0);
    clockLayout->setSpacing(0);

    QLabel *directionLabel = new QLabel(tr("Direction") + ": ");
    directionLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    k->clockCombo = new QComboBox();
    k->clockCombo->addItem(tr("Clockwise"));
    k->clockCombo->addItem(tr("Counterclockwise"));

    clockLayout->addWidget(directionLabel);
    clockLayout->addWidget(k->clockCombo);
    clockLayout->addSpacing(5);

    innerLayout->addLayout(clockLayout);

    setRangeForm();
    innerLayout->addWidget(k->rangePanel);

    innerLayout->addLayout(speedLayout);
    innerLayout->addLayout(speedLayout2);

    innerLayout->addWidget(new TSeparator(Qt::Horizontal));

    k->layout->addWidget(k->innerPanel);

    activeInnerForm(false);
}

void Settings::activeInnerForm(bool enable)
{
    if (enable && !k->innerPanel->isVisible()) {
        k->propertiesDone = true;
        k->innerPanel->show();
    } else {
        k->propertiesDone = false;
        k->innerPanel->hide();
    }
}

void Settings::setRangeForm()
{
    k->rangePanel = new QWidget;
    QBoxLayout *rangeLayout = new QBoxLayout(QBoxLayout::TopToBottom, k->rangePanel);
    rangeLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    rangeLayout->setMargin(0);
    rangeLayout->setSpacing(0);
 
    QLabel *rangeLabel = new QLabel(tr("Degrees Range") + ": ");
    rangeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QLabel *startLabel = new QLabel(tr("Start at") + ": ");
    startLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    k->rangeStart = new QSpinBox;
    k->rangeStart->setEnabled(true);
    k->rangeStart->setMinimum(0);
    k->rangeStart->setMaximum(360);
    connect(k->rangeStart, SIGNAL(valueChanged(int)), this, SLOT(checkRange(int)));

    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->setAlignment(Qt::AlignHCenter);
    startLayout->setMargin(0);
    startLayout->setSpacing(0);
    startLayout->addWidget(startLabel);
    startLayout->addWidget(k->rangeStart);

    QLabel *endLabel = new QLabel(tr("Finish at") + ": ");
    endLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    k->rangeEnd = new QSpinBox;
    k->rangeEnd->setEnabled(true);
    k->rangeEnd->setMinimum(0);
    k->rangeEnd->setMaximum(360);
    connect(k->rangeEnd, SIGNAL(valueChanged(int)), this, SLOT(checkRange(int)));

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->setAlignment(Qt::AlignHCenter);
    endLayout->setMargin(0);
    endLayout->setSpacing(0);
    endLayout->addWidget(endLabel);
    endLayout->addWidget(k->rangeEnd);

    k->rangeLoopBox = new QCheckBox(tr("Loop"), k->rangePanel);

    connect(k->rangeLoopBox, SIGNAL(stateChanged(int)), this, SLOT(updateReverseCheckbox(int)));

    QVBoxLayout *loopLayout = new QVBoxLayout;
    loopLayout->setAlignment(Qt::AlignHCenter);
    loopLayout->setMargin(0);
    loopLayout->setSpacing(0);
    loopLayout->addWidget(k->rangeLoopBox);
    k->reverseLoopBox = new QCheckBox(tr("Loop with Reverse"), k->rangePanel);

    connect(k->reverseLoopBox, SIGNAL(stateChanged(int)), this, SLOT(updateRangeCheckbox(int)));

    QVBoxLayout *reverseLayout = new QVBoxLayout;
    reverseLayout->setAlignment(Qt::AlignHCenter);
    reverseLayout->setMargin(0);
    reverseLayout->setSpacing(0);
    reverseLayout->addWidget(k->reverseLoopBox);

    rangeLayout->addWidget(rangeLabel);
    rangeLayout->addLayout(startLayout);
    rangeLayout->addLayout(endLayout);
    rangeLayout->addSpacing(5);
    rangeLayout->addLayout(loopLayout);
    rangeLayout->addLayout(reverseLayout);

    activeRangeForm(false);
}

void Settings::activeRangeForm(bool enable)
{
    if (enable && !k->rangePanel->isVisible())
        k->rangePanel->show();
    else
        k->rangePanel->hide();
}

// Adding new Tween

void Settings::setParameters(const QString &name, int framesCount, int initFrame)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[Settings::setParameters()]";
        #else
            T_FUNCINFO << "- Adding new tween";
        #endif
    #endif

    k->mode = TupToolPlugin::Add;
    k->input->setText(name);

    activateMode(TupToolPlugin::Selection);

    // Resetting interface
    k->rotationTypeCombo->setCurrentIndex(0);
    k->clockCombo->setCurrentIndex(0);
    k->rangeStart->setValue(0);
    k->rangeEnd->setValue(10);
    k->rangeLoopBox->setChecked(false);
    k->reverseLoopBox->setChecked(false);
    k->degreesPerFrame->setValue(1.0);

    k->apply->setToolTip(tr("Save Tween"));
    k->remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close.png"));
    k->remove->setToolTip(tr("Cancel Tween"));

    initStartCombo(framesCount, initFrame);
}

// Editing current Tween

void Settings::setParameters(TupItemTweener *currentTween)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[Settings::setParameters()]";
        #else
            T_FUNCINFO << "- Editing current tween";
        #endif
    #endif

    setEditMode();
    activateMode(TupToolPlugin::Properties);

    k->input->setText(currentTween->getTweenName());

    k->initFrame->setEnabled(true);
    k->initFrame->setValue(currentTween->getInitFrame());

    k->endFrame->setValue(currentTween->getInitFrame() + currentTween->getFrames());

    // checkFramesRange();
    int end = k->endFrame->value();
    updateRangeFromEnd(end);

    k->rotationTypeCombo->setCurrentIndex(currentTween->tweenRotationType());
    k->degreesPerFrame->setValue(currentTween->tweenRotateSpeed());
    k->clockCombo->setCurrentIndex(currentTween->tweenRotateDirection()); 

    if (currentTween->tweenRotationType() == TupItemTweener::Partial) {
        k->rangeStart->setValue(currentTween->tweenRotateStartDegree());
        k->rangeEnd->setValue(currentTween->tweenRotateEndDegree());

        k->rangeLoopBox->setChecked(currentTween->tweenRotateLoop());
        k->reverseLoopBox->setChecked(currentTween->tweenRotateReverseLoop());
    }
}

void Settings::initStartCombo(int framesCount, int currentIndex)
{
    k->initFrame->clear();
    k->endFrame->clear();

    k->initFrame->setMinimum(1);
    k->initFrame->setMaximum(framesCount);
    k->initFrame->setValue(currentIndex + 1);

    k->endFrame->setMinimum(1);
    k->endFrame->setValue(framesCount);
}

void Settings::setStartFrame(int currentIndex)
{
    k->initFrame->setValue(currentIndex + 1);
    int end = k->endFrame->value();
    if (end < currentIndex+1)
        k->endFrame->setValue(currentIndex + 1);
}

int Settings::startFrame()
{
    return k->initFrame->value() - 1;
}

int Settings::startComboSize()
{
    return k->initFrame->maximum();
}

int Settings::totalSteps()
{
    return k->endFrame->value() - (k->initFrame->value() - 1);
}

void Settings::setEditMode()
{
    k->mode = TupToolPlugin::Edit;
    k->apply->setToolTip(tr("Update Tween"));
    k->remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close_properties.png"));
    k->remove->setToolTip(tr("Close Tween Properties"));
}

void Settings::applyTween()
{
    if (!k->selectionDone) {
        TOsd::self()->display(tr("Info"), tr("You must select at least one object!"), TOsd::Info); 
        #ifdef TUP_DEBUG
            QString msg = "Settings::applyTween() - You must select at least one object!";
            #ifdef Q_OS_WIN
         qDebug() << msg;
            #else
         tError() << msg;
            #endif
        #endif

        return;
    }

    if (!k->propertiesDone) {
        TOsd::self()->display(tr("Info"), tr("You must set Tween properties first!"), TOsd::Error);
        #ifdef TUP_DEBUG
            QString msg = "Settings::applyTween() - You must set Tween properties first!";
            #ifdef Q_OS_WIN
         qDebug() << msg;
            #else
         tError() << msg;
            #endif
        #endif
        return;
    }

    if (k->rotationType == TupItemTweener::Partial) {
        int start = k->rangeStart->value();
        int end = k->rangeEnd->value();
        if (start == end) {
            TOsd::self()->display(tr("Info"), tr("Angle range must be greater than 0!"), TOsd::Error);
            #ifdef TUP_DEBUG
         QString msg = "Settings::applyTween() - Angle range must be greater than 0!";
         #ifdef Q_OS_WIN
             qDebug() << msg;
         #else
             tError() << msg;
         #endif
            #endif
            return;
        }

        int range = abs(start - end); 
        if (range < k->degreesPerFrame->value()) { 
            TOsd::self()->display(tr("Info"), tr("Angle range must be greater than Speed!"), TOsd::Error);
            #ifdef TUP_DEBUG
         QString msg = "Settings::applyTween() - Angle range must be greater than Speed!";
         #ifdef Q_OS_WIN
             qDebug() << msg;
         #else
             tError() << msg;
         #endif
            #endif
            return;
        }
    }

    // SQA: Verify Tween is really well applied before call setEditMode!
    setEditMode();

    if (!k->initFrame->isEnabled())
        k->initFrame->setEnabled(true);

    checkFramesRange();
    emit clickedApplyTween();
}

void Settings::notifySelection(bool flag)
{
    k->selectionDone = flag;
}

QString Settings::currentTweenName() const
{
    QString tweenName = k->input->text();
    if (tweenName.length() > 0)
        k->input->setFocus();

    return tweenName;
}

void Settings::emitOptionChanged(int option)
{
    switch (option) {
        case 0:
        {
            activeInnerForm(false);
            emit clickedSelect();
        }
        break;
        case 1:
        {
            if (k->selectionDone) {
         activeInnerForm(true);
         emit clickedDefineAngle();
            } else {
         k->options->setCurrentIndex(0);
         TOsd::self()->display(tr("Info"), tr("Select objects for Tweening first!"), TOsd::Info);
         #ifdef TUP_DEBUG
             QString msg = "Settings::emitOptionChanged() - You must set Tween properties first!";
             #ifdef Q_OS_WIN
                 qDebug() << msg;
             #else
                 tError() << msg;
             #endif
         #endif
            }
        }
    }
}

QString Settings::tweenToXml(int currentScene, int currentLayer, int currentFrame, QPointF point)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("tweening");
    root.setAttribute("name", currentTweenName());
    root.setAttribute("type", TupItemTweener::Rotation);
    root.setAttribute("initFrame", currentFrame);
    root.setAttribute("initLayer", currentLayer);
    root.setAttribute("initScene", currentScene);
    
    // checkFramesRange();
    root.setAttribute("frames", k->totalSteps);

    root.setAttribute("origin", QString::number(point.x()) + "," + QString::number(point.y()));
    root.setAttribute("rotationType", k->rotationType);
    double speed = k->degreesPerFrame->value();

    root.setAttribute("rotateSpeed", QString::number(speed));

    int direction = k->clockCombo->currentIndex();
    root.setAttribute("rotateDirection", direction);

    if (k->rotationType == TupItemTweener::Continuos) {
        double angle = 0;
        for (int i=0; i < k->totalSteps; i++) {
            TupTweenerStep *step = new TupTweenerStep(i);
            step->setRotation(angle);

            root.appendChild(step->toXml(doc));
            if (direction == TupItemTweener::Clockwise)
         angle += speed;
            else
         angle -= speed;
        }
    } else if (k->rotationType == TupItemTweener::Partial) {
        bool loop = k->rangeLoopBox->isChecked();
        if (loop)
            root.setAttribute("rotateLoop", "1");
        else
            root.setAttribute("rotateLoop", "0");

        int start = k->rangeStart->value();
        root.setAttribute("rotateStartDegree", start);

        int end = k->rangeEnd->value();
        root.setAttribute("rotateEndDegree", end);

        bool reverse = k->reverseLoopBox->isChecked();
        if (reverse)
            root.setAttribute("rotateReverseLoop", "1");
        else
            root.setAttribute("rotateReverseLoop", "0");

        double angle = start;
        bool token = false;

        double distance = 0;
        if (direction == TupItemTweener::Clockwise) {
            if (start > end)
                distance = 360 - (start - end);
            else
                distance = end - start;
        } else { // CounterClockwise
            if (start > end)
                distance = start - end;
            else
                distance = 360 - (end - start);
        }

        double counter = 0; 
        double go = distance;
        double back = distance - (2*speed);

        for (int i=0; i < k->totalSteps; i++) {
             TupTweenerStep *step = new TupTweenerStep(i);
             step->setRotation(angle);
             root.appendChild(step->toXml(doc));

             if (!token) { // going on initial direction
                 if (counter < distance) {
                     if (direction == TupItemTweener::Clockwise)
                         angle += speed;
                     else
                         angle -= speed;

                     if (end < start) {
                         if (angle >= 360)
                             angle = angle - 360;
                     }
                 }
             } else { // returning back
                 if (counter < distance) {
                     if (direction == TupItemTweener::Clockwise)
                         angle -= speed;
                     else
                         angle += speed;

                     if (end < start) {
                         if (angle < 0)
                             angle = 360 - fabs(angle);
                     }
                 }
             }

             if (reverse) {
                 if (counter >= distance) {
                     token = !token;
                     counter = 0;

                     if (direction == TupItemTweener::Clockwise) {
                         angle -= speed;
                         if (angle < 0)
                             angle = 360 - fabs(angle);
                     } else {
                         angle += speed;
                         if (angle >= 360)
                             angle = angle - 360;
                     }

                     if (token)
                         distance = back;
                     else
                         distance = go;
                 } else {
                     counter += speed;
                 }
             } else if (loop && counter >= distance) {
                 angle = start;
                 counter = 0;
             } else {
                 counter += speed;
             }
        }
    }
    doc.appendChild(root);

    return doc.toString();
}

void Settings::activateMode(TupToolPlugin::EditMode mode)
{
    k->options->setCurrentIndex(mode);
}

void Settings::refreshForm(int type)
{
    if (type == 0) {
        k->rotationType = TupItemTweener::Continuos;
        activeRangeForm(false);
    } else {
        k->rotationType = TupItemTweener::Partial;
        activeRangeForm(true);
    }
}

/*
void Settings::checkTopLimit(int index)
{
    Q_UNUSED(index);
    checkFramesRange();
}
*/

/*
void Settings::updateLastFrame()
{
    int end = k->initFrame->value() + k->totalSteps - 1;
    k->endFrame->setValue(end);
}
*/

void Settings::checkFramesRange()
{
    int begin = k->initFrame->value();
    int end = k->endFrame->value();
       
    if (begin > end) {
        // k->endFrame->setValue(k->endFrame->maximum() - 1);
        // end = k->endFrame->value();
        k->initFrame->blockSignals(true);
        k->endFrame->blockSignals(true);
        int tmp = end;
        end = begin;
        begin = tmp;
        k->initFrame->setValue(begin);
        k->endFrame->setValue(end);
        k->initFrame->blockSignals(false);
        k->endFrame->blockSignals(false);
    }

    k->totalSteps = end - begin + 1;
    k->totalLabel->setText(tr("Frames Total") + ": " + QString::number(k->totalSteps));
}

void Settings::updateRangeCheckbox(int state)
{
    Q_UNUSED(state);

    if (k->reverseLoopBox->isChecked() && k->rangeLoopBox->isChecked())
        k->rangeLoopBox->setChecked(false);
}

void Settings::updateReverseCheckbox(int state)
{
    Q_UNUSED(state);

    if (k->reverseLoopBox->isChecked() && k->rangeLoopBox->isChecked())
        k->reverseLoopBox->setChecked(false);
}

/*
void Settings::updateTotalSteps(const QString &text)
{
    Q_UNUSED(text);
    checkFramesRange();
}
*/

void Settings::checkRange(int index)
{
    Q_UNUSED(index);

    int start = k->rangeStart->value();
    int end = k->rangeEnd->value();

    if (start == end) {
        if (k->rangeEnd->value() == 360)
            k->rangeStart->setValue(k->rangeStart->value() - 1);
        else
            k->rangeEnd->setValue(k->rangeEnd->value() + 1);
    }
}

void Settings::updateRangeFromInit(int begin)
{
    int end = k->endFrame->value();
    k->totalSteps = end - begin + 1;
    k->totalLabel->setText(tr("Frames Total") + ": " + QString::number(k->totalSteps));
}

void Settings::updateRangeFromEnd(int end)
{
    int begin = k->initFrame->value();
    k->totalSteps = end - begin + 1;
    k->totalLabel->setText(tr("Frames Total") + ": " + QString::number(k->totalSteps));
}

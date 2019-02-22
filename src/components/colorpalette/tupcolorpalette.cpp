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

#include "tupcolorpalette.h"

#include "timagebutton.h"
#include "tseparator.h"
#include "tupmodulewidgetbase.h"
#include "ticon.h"
#include "tcolorcell.h"
#include "tconfig.h"
#include "tuppaintareaevent.h"
#include "tvhbox.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFrame>
#include <QToolTip>
#include <QComboBox>
#include <QGroupBox>
#include <QMenu>
#include <QTabWidget>

struct TupColorPalette::Private
{
    QSplitter *splitter;
    QTabWidget *tab;

    TupViewColorCells *paletteContainer;
    TupColorForm *colorForm;
    TupColorPicker *colorPickerArea;
    TSlider *luminancePicker;
    TupGradientCreator *gradientManager;

    QLineEdit *htmlField;
    QLineEdit *bgHtmlField;

    QBrush currentContourBrush;
    QBrush currentFillBrush;

    TColorCell *contourColor;
    TColorCell *fillColor;
    TColorCell *bgColor;

    bool flagGradient;
    BrushType type;

    TColorCell::FillType currentSpace;
    TupColorPalette::BrushType fgType;
    TupColorPalette::BrushType bgType; 
};

TupColorPalette::TupColorPalette(QWidget *parent) : TupModuleWidgetBase(parent), k(new Private)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupColorPalette()]";
        #else
            TINIT;
        #endif
    #endif

    k->currentSpace = TColorCell::Contour;
    TCONFIG->beginGroup("ColorPalette");
    TCONFIG->setValue("CurrentColorMode", 0);

    k->currentContourBrush = Qt::black;
    k->currentFillBrush = Qt::transparent;
    k->flagGradient = true;

    setWindowTitle(tr("Color Palette"));
    setWindowIcon(QPixmap(THEME_DIR + "icons/color_palette.png"));

    k->splitter = new QSplitter(Qt::Vertical, this);

    k->tab = new QTabWidget;
    connect(k->tab, SIGNAL(currentChanged(int)), this, SLOT(updateColorType(int)));

    setupColorDisplay();

    addChild(k->splitter);

    setupMainPalette();
    setupColorChooser();
    setupGradientManager();

    k->tab->setPalette(palette());
    k->tab->setMinimumHeight(320);
    k->splitter->addWidget(k->tab);

    setMinimumWidth(316);
}

TupColorPalette::~TupColorPalette()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupColorPalette()]";
        #else
            TEND;
        #endif
    #endif

    delete k->paletteContainer;
    k->paletteContainer = nullptr;

    delete k->colorForm;
    k->colorForm = nullptr;

    delete k->colorPickerArea;
    k->colorPickerArea = nullptr;

    delete k->luminancePicker;
    k->luminancePicker = nullptr;

    delete k->gradientManager;
    k->gradientManager = nullptr;

    delete k->contourColor;
    k->contourColor = nullptr;

    delete k->fillColor;
    k->fillColor = nullptr;

    delete k->bgColor;
    k->bgColor = nullptr;

    delete k;
}

void TupColorPalette::setupColorDisplay()
{
    QFrame *topPanel = new QFrame(this);
    QBoxLayout *generalLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    generalLayout->setMargin(0);
    topPanel->setLayout(generalLayout);

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(1);

    mainLayout->addWidget(new QWidget());

    TImageButton *changeButton = new TImageButton(QIcon(QPixmap(THEME_DIR + "icons/exchange_colors.png")), 20, this, true);
    changeButton->setToolTip(tr("Exchange colors"));
    connect(changeButton, SIGNAL(clicked()), this, SLOT(switchColors()));
    mainLayout->addWidget(changeButton);
    mainLayout->setSpacing(5);

    QBoxLayout *listLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QLabel *contourLabel = new QLabel(tr("Contour"));
    QLabel *fillLabel = new QLabel(tr("Fill"));

    QSize cellSize(25, 25);
    QColor contourColor(0, 0, 0);
    QBrush contourBrush = QBrush(Qt::black);
    k->contourColor = new TColorCell(TColorCell::Contour, contourBrush, cellSize);
    k->contourColor->setChecked(true);
    connect(k->contourColor, SIGNAL(clicked(TColorCell::FillType)), this, SLOT(updateColorMode(TColorCell::FillType)));

    QBrush fillBrush = QBrush(Qt::transparent);
    k->fillColor = new TColorCell(TColorCell::Inner, fillBrush, cellSize);
    connect(k->fillColor, SIGNAL(clicked(TColorCell::FillType)), this, SLOT(updateColorMode(TColorCell::FillType)));

    QBoxLayout *contourLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    contourLayout->addWidget(k->contourColor);
    contourLayout->addWidget(contourLabel);

    QBoxLayout *fillLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    fillLayout->addWidget(k->fillColor);
    fillLayout->addWidget(fillLabel);

    listLayout->addLayout(contourLayout);
    listLayout->setSpacing(5);
    listLayout->addLayout(fillLayout);

    mainLayout->addLayout(listLayout);

    TImageButton *resetButton = new TImageButton(QIcon(QPixmap(THEME_DIR + "icons/reset_colors.png")), 15, this, true);
    resetButton->setToolTip(tr("Reset colors"));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(init()));
    mainLayout->addWidget(resetButton);

    mainLayout->addWidget(new QWidget());

    QLabel *htmlLabel = new QLabel(tr("HTML"), topPanel);
    htmlLabel->setMaximumWidth(50);
    htmlLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    mainLayout->addWidget(htmlLabel);

    k->htmlField = new QLineEdit(topPanel);
    k->htmlField->setMaximumWidth(70);
    k->htmlField->setText("#000000");
    connect(k->htmlField, SIGNAL(editingFinished()), this, SLOT(updateColorFromHTML()));
    mainLayout->addWidget(k->htmlField);

    mainLayout->addWidget(new QWidget());

    generalLayout->addLayout(mainLayout);
    generalLayout->addWidget(new QWidget());
    generalLayout->setAlignment(mainLayout, Qt::AlignHCenter|Qt::AlignVCenter);

    generalLayout->addWidget(new TSeparator(Qt::Horizontal));    

    QBoxLayout *bgLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    bgLayout->setMargin(0);
    bgLayout->setSpacing(1);

    bgLayout->addWidget(new QWidget());

    QBrush bgBrush = QBrush(Qt::white);
    k->bgColor = new TColorCell(TColorCell::Background, bgBrush, cellSize);
    connect(k->bgColor, SIGNAL(clicked(TColorCell::FillType)), this, SLOT(updateColorMode(TColorCell::FillType)));
    bgLayout->addWidget(k->bgColor);

    bgLayout->setSpacing(5);

    QLabel *bgLabel = new QLabel(tr("Background"), topPanel);
    bgLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    bgLayout->addWidget(bgLabel);

    QWidget *space1 = new QWidget();
    space1->setFixedWidth(10);
    bgLayout->addWidget(space1);

    TImageButton *resetBgButton = new TImageButton(QIcon(QPixmap(THEME_DIR + "icons/reset_bg.png")), 15, this, true);
    resetBgButton->setToolTip(tr("Reset background"));
    connect(resetBgButton, SIGNAL(clicked()), this, SLOT(initBg()));
    bgLayout->addWidget(resetBgButton);

    QWidget *space2 = new QWidget();
    space2->setFixedWidth(10);
    bgLayout->addWidget(space2);

    QLabel *bgHtmlLabel = new QLabel(tr("HTML"), topPanel);
    bgHtmlLabel->setMaximumWidth(50);
    bgHtmlLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    bgLayout->addWidget(bgHtmlLabel);

    bgLayout->setSpacing(5);

    k->bgHtmlField = new QLineEdit();
    k->bgHtmlField->setMaximumWidth(70);
    k->bgHtmlField->setText("#ffffff");
    connect(k->bgHtmlField, SIGNAL(editingFinished()), this, SLOT(updateBgColorFromHTML()));
    bgLayout->addWidget(k->bgHtmlField);

    bgLayout->addWidget(new QWidget());

    generalLayout->addLayout(bgLayout);
    generalLayout->setAlignment(bgLayout, Qt::AlignHCenter);

    generalLayout->addWidget(new QWidget());

    addChild(topPanel);
}

void TupColorPalette::updateColorMode(TColorCell::FillType type)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupColorPalette::updateColorMode()] - type: " << type;
        #else
            T_FUNCINFOX("tools") << type;
        #endif
    #endif

    QBrush brush;
    QColor color;

    if (type == TColorCell::Background) {
        k->paletteContainer->enableTransparentColor(false);
        k->currentSpace = TColorCell::Background;
        brush = k->bgColor->brush();
        color = brush.color();
        k->bgHtmlField->setText(color.name());

        if (k->contourColor->isChecked())
            k->contourColor->setChecked(false);
        if (k->fillColor->isChecked())
            k->fillColor->setChecked(false);
    } else {
        k->paletteContainer->enableTransparentColor(true);
        k->paletteContainer->resetBasicPanel();
        if (k->bgColor->isChecked())
            k->bgColor->setChecked(false);

        if (type == TColorCell::Contour) {
            k->currentSpace = TColorCell::Contour;
            brush = k->contourColor->brush();
            if (k->fillColor->isChecked()) 
                k->fillColor->setChecked(false);
        } else if (type == TColorCell::Inner) {
            k->currentSpace = TColorCell::Inner;
            brush = k->fillColor->brush();
            if (k->contourColor->isChecked())
                k->contourColor->setChecked(false);
        }

        color = brush.color();
        k->htmlField->setText(color.name());
        TCONFIG->beginGroup("ColorPalette");
        TCONFIG->setValue("CurrentColorMode", type);

        emit colorSpaceChanged(type);
    }

    if (k->fgType == Solid && k->tab->currentIndex() != 0) {
        k->tab->setCurrentIndex(0);
    } else if (k->fgType == Gradient && k->tab->currentIndex() != 1) {
        k->tab->setCurrentIndex(1);
    }

    updateLuminancePicker(color);
    k->colorForm->setColor(color);
    k->gradientManager->setCurrentColor(color);
}

void TupColorPalette::setupMainPalette()
{
    // Palettes
    k->paletteContainer = new TupViewColorCells(k->splitter);
    connect(k->paletteContainer, SIGNAL(colorSelected(const QBrush&)), this, SLOT(updateColorFromPalette(const QBrush&)));

    k->splitter->addWidget(k->paletteContainer);
}

void TupColorPalette::setupColorChooser()
{
    QFrame *colorMixer = new QFrame;
    colorMixer->setFrameStyle(QFrame::Box | QFrame::Sunken);

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    colorMixer->setLayout(mainLayout);

    k->colorPickerArea = new TupColorPicker(colorMixer);
    connect(k->colorPickerArea, SIGNAL(newColor(int, int)), this, SLOT(setHS(int, int)));

    k->luminancePicker = new TSlider(Qt::Horizontal, TSlider::Color, QColor(0, 0, 0), QColor(255, 255, 255));
    connect(k->luminancePicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(syncColor(const QColor &)));
    k->luminancePicker->setRange(0, 100);
    k->luminancePicker->setValue(100);

    k->colorForm = new TupColorForm;
    connect(k->colorForm, SIGNAL(brushChanged(const QBrush&)), this, SLOT(updateColorFromDisplay(const QBrush&)));

    mainLayout->addWidget(k->colorPickerArea);
    mainLayout->setAlignment(k->colorPickerArea, Qt::AlignHCenter);

    mainLayout->addWidget(k->luminancePicker);
    mainLayout->setAlignment(k->luminancePicker, Qt::AlignHCenter);

    mainLayout->addWidget(k->colorForm);
    mainLayout->setAlignment(k->colorForm, Qt::AlignHCenter);

    mainLayout->addStretch(2);

    k->tab->addTab(colorMixer, tr("Color Mixer"));
}

void TupColorPalette::setupGradientManager()
{
    k->gradientManager = new TupGradientCreator(this);
    // connect(k->gradientManager, SIGNAL(gradientChanged(const QBrush&)), this, SLOT(updateGradientColor(const QBrush &)));

    k->tab->addTab(k->gradientManager, tr("Gradients"));
    // SQA: Temporary code
    k->tab->setTabEnabled(1, false);
}

void TupColorPalette::setColorOnAppFromHTML(const QBrush& brush)
{
    QColor color = brush.color();

    if (color.isValid()) {
        if (k->type == Gradient)
            k->gradientManager->setCurrentColor(color);

        k->colorPickerArea->setColor(color.hue(), color.saturation());
        k->paletteContainer->setColor(brush);
        k->colorForm->setColor(color);

        // if (k->type == Solid)
        //     k->outlineAndFillColors->setCurrentColor(color);
    } else if (brush.gradient()) {
               QGradient gradient(*brush.gradient());
               // changeBrushType(tr("Gradient"));

               k->paletteContainer->setColor(gradient);
               // k->outlineAndFillColors->setCurrentColor(gradient);
               if (sender() != k->gradientManager)
                   k->gradientManager->setGradient(gradient);

               // SQA: Gradient issue pending for revision
               // tFatal() << "TupColorPalette::setColor() - Sending gradient value!";
               // TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBrush, brush);
               // emit paintAreaEventTriggered(&event);
               // return;
    }

    if (k->currentSpace == TColorCell::Background) {
        k->bgColor->setBrush(brush);

        TupPaintAreaEvent bgEvent(TupPaintAreaEvent::ChangeBgColor, k->bgColor->color());
        emit paintAreaEventTriggered(&bgEvent);
        return;
    }

    if (k->currentSpace == TColorCell::Contour) {
        k->contourColor->setBrush(brush); 

        TupPaintAreaEvent contourEvent(TupPaintAreaEvent::ChangePenColor, k->contourColor->color());
        emit paintAreaEventTriggered(&contourEvent);
        return;
    } 

    if (k->currentSpace == TColorCell::Inner) { 
        k->fillColor->setBrush(brush);

        TupPaintAreaEvent fillEvent(TupPaintAreaEvent::ChangeBrush, brush);
        emit paintAreaEventTriggered(&fillEvent);
    }
}

void TupColorPalette::setGlobalColors(const QBrush &brush)
{
    if (k->currentSpace == TColorCell::Background) {
        k->bgColor->setBrush(brush);
        k->bgHtmlField->setText(brush.color().name());

        TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBgColor, brush.color());
        emit paintAreaEventTriggered(&event);
    } else {
        if (k->currentSpace == TColorCell::Contour) {
            if (brush.color() == Qt::transparent) {
                if (k->fillColor->color() == Qt::transparent) {
                    QBrush black(Qt::black);
                    k->fillColor->setBrush(black);
                    k->currentFillBrush = black;

                    TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBrush, black);
                    emit paintAreaEventTriggered(&event);
                }
            }

            k->contourColor->setBrush(brush);
            k->currentContourBrush = brush;

            TupPaintAreaEvent event(TupPaintAreaEvent::ChangePenColor, brush.color());
            emit paintAreaEventTriggered(&event);
        } else {
            if (brush.color() == Qt::transparent) {
                if (k->contourColor->color() == Qt::transparent) {
                    QBrush black(Qt::black);
                    k->contourColor->setBrush(black);
                    k->currentContourBrush = black;

                    TupPaintAreaEvent event(TupPaintAreaEvent::ChangePenColor, black);
                    emit paintAreaEventTriggered(&event);
                }
            }

            k->fillColor->setBrush(brush);
            k->currentFillBrush = brush;

            TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBrush, brush);
            emit paintAreaEventTriggered(&event);
        }
        k->htmlField->setText(brush.color().name());
    }
}

void TupColorPalette::updateColorFromPalette(const QBrush &brush)
{
    k->colorPickerArea->clearSelection();

    setGlobalColors(brush);
    QColor color = brush.color();
    updateLuminancePicker(color);
    k->colorForm->setColor(color);
    k->gradientManager->setCurrentColor(color);
}

void TupColorPalette::updateColorFromDisplay(const QBrush &brush)
{
    setGlobalColors(brush);
    QColor color = brush.color();
    k->colorPickerArea->setColor(color.hue(), color.saturation());
    updateLuminancePicker(color);
}

void TupColorPalette::updateGradientColor(const QBrush &brush)
{
    setGlobalColors(brush);
}

void TupColorPalette::syncColor(const QColor &color)
{
    setGlobalColors(QBrush(color));
    k->colorForm->setColor(color);
}

void TupColorPalette::setHS(int hue, int saturation)
{
    k->paletteContainer->clearSelection();
    k->paletteContainer->resetBasicPanel();

    int luminance = 255;
    if (hue == 0 && saturation == 0)
        luminance = 0;

    QColor color;
    color.setHsv(hue, saturation, luminance, 255);

    setGlobalColors(QBrush(color));
    updateLuminancePicker(color);
    k->colorForm->setColor(color);
}

void TupColorPalette::updateColorFromHTML()
{
    QColor currentColor;
    if (k->currentSpace == TColorCell::Background) {
        k->bgColor->setChecked(false);
        k->currentSpace = TColorCell::Contour;
        k->contourColor->setChecked(true);
        currentColor = k->contourColor->color();
    } else if (k->currentSpace == TColorCell::Contour) {
        currentColor = k->contourColor->color(); 
    } else if (k->currentSpace == TColorCell::Inner) {
        currentColor = k->fillColor->color();
    }

    QString colorCode = k->htmlField->text();
    QColor color(colorCode);

    if (color != currentColor) {
        color.setAlpha(255);
        setColorOnAppFromHTML(color);
    }
}

void TupColorPalette::updateBgColorFromHTML()
{
    if (k->currentSpace != TColorCell::Background) {
        if (k->contourColor->isChecked())
            k->contourColor->setChecked(false);
        if (k->fillColor->isChecked())
            k->fillColor->setChecked(false);

        k->currentSpace = TColorCell::Background;
        k->bgColor->setChecked(true);
    }

    QColor currentColor = k->bgColor->color();
    QString colorCode = k->bgHtmlField->text();
    QColor color(colorCode);

    if (color != currentColor) {
        color.setAlpha(255);
        setColorOnAppFromHTML(color);
    }
}

QPair<QColor, QColor> TupColorPalette::color()
{
    QPair<QColor, QColor> colors;
    colors.first = k->contourColor->color();
    colors.second = k->fillColor->color();

    return colors;
}

void TupColorPalette::parsePaletteFile(const QString &file)
{
    k->paletteContainer->readPaletteFile(file);
}

void TupColorPalette::init()
{
    if (k->bgColor->isChecked())
        k->bgColor->setChecked(false);

    k->currentSpace = TColorCell::Contour;

    QColor contourColor = Qt::black;
    k->currentContourBrush = QBrush(contourColor);
    k->htmlField->setText("#000000");

    QBrush fillBrush = QBrush(Qt::transparent);
    k->currentFillBrush = fillBrush;

    k->contourColor->setBrush(k->currentContourBrush);
    k->fillColor->setBrush(fillBrush);

    if (k->fillColor->isChecked())
        k->fillColor->setChecked(false);
    k->contourColor->setChecked(true);

    k->paletteContainer->clearSelection();

    k->colorPickerArea->setColor(contourColor.hue(), contourColor.saturation());

    if (!k->luminancePicker->isEnabled())
        k->luminancePicker->setEnabled(true);

    blockSignals(true);
    k->luminancePicker->setColors(Qt::black, Qt::white);
    k->luminancePicker->setValue(0);
    k->colorForm->setColor(contourColor);
    k->gradientManager->setCurrentColor(Qt::white);
    blockSignals(false);

    emit colorSpaceChanged(TColorCell::Contour);

    TupPaintAreaEvent fillEvent(TupPaintAreaEvent::ChangeBrush, k->currentFillBrush);
    emit paintAreaEventTriggered(&fillEvent);

    TupPaintAreaEvent event(TupPaintAreaEvent::ChangePenColor, contourColor);
    emit paintAreaEventTriggered(&event);
}

void TupColorPalette::setBgColor(const QColor &color)
{
    QBrush brush(color);
    k->bgColor->setBrush(brush);
}

void TupColorPalette::initBg()
{
    QBrush brush(Qt::white);
    k->bgColor->setBrush(brush);
    k->bgColor->setChecked(true);
    updateColorMode(TColorCell::Background);

    k->paletteContainer->clearSelection();

    TupPaintAreaEvent event(TupPaintAreaEvent::ChangeBgColor, brush.color());
    emit paintAreaEventTriggered(&event);
}

void TupColorPalette::switchColors()
{
    k->paletteContainer->clearSelection();

    QBrush tmp = k->currentContourBrush;
    k->currentContourBrush = k->currentFillBrush;
    k->currentFillBrush = tmp;

    if (k->bgColor->isChecked())
        k->bgColor->setChecked(false);
    k->contourColor->setBrush(k->currentContourBrush); 
    k->fillColor->setBrush(k->currentFillBrush);

    QColor color;
    if (k->currentSpace == TColorCell::Contour)
        color = k->contourColor->color();
    else
        color = k->fillColor->color();

    blockSignals(true);
    k->htmlField->setText(color.name());
    k->colorPickerArea->setColor(color.hue(), color.saturation());
    updateLuminancePicker(color);
    k->colorForm->setColor(color);
    blockSignals(false);

    TupPaintAreaEvent event = TupPaintAreaEvent(TupPaintAreaEvent::ChangeBrush, k->currentFillBrush);
    emit paintAreaEventTriggered(&event);

    event = TupPaintAreaEvent(TupPaintAreaEvent::ChangePenColor, k->currentContourBrush.color());
    emit paintAreaEventTriggered(&event);

    k->currentSpace = TColorCell::Contour;
}

void TupColorPalette::updateColorType(int index)
{
    if (index == TupColorPalette::Solid) {
        if (k->currentSpace == TColorCell::Contour)
            k->fgType = Solid; 
        else
            k->bgType = Solid;
    } else {
        if (k->currentSpace == TColorCell::Inner) 
            k->fgType = Gradient;
        else
            k->bgType = Gradient;
    }
}

void TupColorPalette::updateLuminancePicker(const QColor &color)
{
    if (color == Qt::transparent) {
        k->luminancePicker->setEnabled(false);
        return;
    } 

    if (!k->luminancePicker->isEnabled())
        k->luminancePicker->setEnabled(true);

    if (color != Qt::black) {
        k->luminancePicker->setColors(Qt::black, color);
        k->luminancePicker->setValue(100);
    } else {
        k->luminancePicker->setColors(Qt::black, Qt::white);
        k->luminancePicker->setValue(0);
    }
}

void TupColorPalette::updateContourColor(const QColor &color)
{
    if (k->bgColor->isChecked())
        k->bgColor->setChecked(false);

    if (k->fillColor->isChecked())
        k->fillColor->setChecked(false);

    if (!k->contourColor->isChecked())
        k->contourColor->setChecked(true);

    if (color != k->contourColor->color()) {
        k->contourColor->setBrush(QBrush(color));
        updateColorMode(TColorCell::Contour);
    }
}

void TupColorPalette::updateFillColor(const QColor &color)
{
    if (k->bgColor->isChecked())
        k->bgColor->setChecked(false);

    if (k->contourColor->isChecked())
        k->contourColor->setChecked(false);

    if (!k->fillColor->isChecked())
        k->fillColor->setChecked(true);

    if (color != k->fillColor->color()) {
        k->fillColor->setBrush(QBrush(color));
        updateColorMode(TColorCell::Inner);
    }
}

void TupColorPalette::updateBgColor(const QColor &color)
{
    if (k->contourColor->isChecked())
        k->contourColor->setChecked(false);

    if (k->fillColor->isChecked())
        k->fillColor->setChecked(false);

    if (!k->bgColor->isChecked())
        k->bgColor->setChecked(true);

    if (color != k->bgColor->color()) {
        k->bgColor->setBrush(QBrush(color));
        updateColorMode(TColorCell::Background);
    }
}

void TupColorPalette::clickFillButton()
{
    if (!k->fillColor->isChecked())
        k->fillColor->click();
}

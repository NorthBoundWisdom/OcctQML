#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#include "OcctQtViewer.h"

MyMainWindow::MyMainWindow()
    : myViewer(nullptr)
{
    setUpMainBar();
    setUpViewer();
}

void MyMainWindow::setUpMainBar()
{
    // menu bar with Quit item
    QMenuBar *aMenuBar = new QMenuBar();
    QMenu *aMenuWindow = aMenuBar->addMenu("&File");
    {
        QAction *anActionSplit = new QAction(aMenuWindow);
        anActionSplit->setText("Split Views");
        aMenuWindow->addAction(anActionSplit);
        connect(anActionSplit, &QAction::triggered,
                [this]()
                {
                    if (!myViewer->View()->Subviews().IsEmpty())
                    {
                        // remove subviews
                        myViewer->View()->View()->SetSubviewComposer(false);
                        NCollection_Sequence<Handle(V3d_View)> aSubviews =
                            myViewer->View()->Subviews();
                        for (Handle(V3d_View) aSubviewIter : aSubviews)
                        {
                            aSubviewIter->Remove();
                        }
                        myViewer->OnSubviewChanged(myViewer->Context(), nullptr, myViewer->View());
                    }
                    else
                    {
                        // create two subviews splitting window horizontally
                        myViewer->View()->View()->SetSubviewComposer(true);

                        Handle(V3d_View) aSubView1 = new V3d_View(myViewer->Viewer());
                        aSubView1->SetImmediateUpdate(false);
                        aSubView1->SetWindow(myViewer->View(), Graphic3d_Vec2d(0.5, 1.0),
                                             Aspect_TOTP_LEFT_UPPER, Graphic3d_Vec2d(0.0, 0.0));

                        Handle(V3d_View) aSubView2 = new V3d_View(myViewer->Viewer());
                        aSubView2->SetImmediateUpdate(false);
                        aSubView2->SetWindow(myViewer->View(), Graphic3d_Vec2d(0.5, 1.0),
                                             Aspect_TOTP_LEFT_UPPER, Graphic3d_Vec2d(0.5, 0.0));

                        myViewer->OnSubviewChanged(myViewer->Context(), nullptr, aSubView1);
                    }
                    myViewer->View()->Invalidate();
                    myViewer->update();
                });
    }
    {
        QAction *anActionQuit = new QAction(aMenuWindow);
        anActionQuit->setText("Quit");
        aMenuWindow->addAction(anActionQuit);
        connect(anActionQuit, &QAction::triggered, [this]() { close(); });
    }
    setMenuBar(aMenuBar);
}

void MyMainWindow::setUpViewer()
{
    bool isCoreProfile = false;
#if defined(_WIN32)
    isCoreProfile = true;
#elif defined(__APPLE__)
    isCoreProfile = true;
#else
    isCoreProfile = false;
#endif

    // 3D Viewer and some controls on top of it
    myViewer = new OcctQtViewer(isCoreProfile);
    QVBoxLayout *aLayout = new QVBoxLayout(myViewer);
    aLayout->setDirection(QBoxLayout::BottomToTop);
    aLayout->setAlignment(Qt::AlignBottom);
    {
        QPushButton *aQuitBtn = new QPushButton("About");
        aLayout->addWidget(aQuitBtn);
    }
    {
        QWidget *aSliderBox = new QWidget();
        QHBoxLayout *aSliderLayout = new QHBoxLayout(aSliderBox);
        {
            QLabel *aSliderLabel = new QLabel("Background");
            aSliderLabel->setStyleSheet(
                "QLabel { background-color: rgba(0, 0, 0, 0); color: white; }");
            aSliderLabel->setGeometry(50, 50, 50, 50);
            aSliderLabel->adjustSize();
            aSliderLayout->addWidget(aSliderLabel);
        }
        {
            QSlider *aSlider = new QSlider(Qt::Horizontal);
            aSlider->setRange(0, 255);
            aSlider->setSingleStep(1);
            aSlider->setPageStep(15);
            aSlider->setTickInterval(15);
            aSlider->setTickPosition(QSlider::TicksRight);
            aSlider->setValue(0);
            aSliderLayout->addWidget(aSlider);
            connect(aSlider, &QSlider::valueChanged,
                    [this](int theValue)
                    {
                        const double aVal = static_cast<double>(theValue) / 255.0;
                        const Quantity_Color aColor(aVal, aVal, aVal, Quantity_TOC_sRGB);

                        for (Handle(V3d_View) aSubviewIter : myViewer->View()->Subviews())
                        {
                            aSubviewIter->SetBgGradientColors(aColor, Quantity_NOC_BLACK,
                                                              Aspect_GradientFillMethod_Elliptical);
                            aSubviewIter->Invalidate();
                        }
                        // myViewer->View()->SetBackgroundColor (aColor);
                        myViewer->View()->SetBgGradientColors(aColor, Quantity_NOC_BLACK,
                                                              Aspect_GradientFillMethod_Elliptical);
                        myViewer->View()->Invalidate();
                        myViewer->update();
                    });
        }
        aLayout->addWidget(aSliderBox);
    }
    setCentralWidget(myViewer);
}

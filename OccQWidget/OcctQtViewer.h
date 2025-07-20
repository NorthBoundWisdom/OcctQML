// Copyright (c) 2021 OPEN CASCADE SAS
//
// This file is part of the examples of the Open CASCADE Technology software library.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE

#ifndef OCCQT_VIEWER_H
#define OCCQT_VIEWER_H

#include <QOpenGLWidget>

#include <AIS_InteractiveContext.hxx>
#include <AIS_ViewController.hxx>
#include <Standard_WarningsDisable.hxx>
#include <Standard_WarningsRestore.hxx>
#include <V3d_View.hxx>

class AIS_ViewCube;

//! OCCT 3D View.
class OcctQtViewer : public QOpenGLWidget, public AIS_ViewController
{
    Q_OBJECT

public:
    //! Main constructor.
    OcctQtViewer(bool theIsCoreProfile, QWidget *theParent = nullptr);

    //! Destructor.
    ~OcctQtViewer() override;

    //! Return Viewer.
    const Handle(V3d_Viewer) & Viewer() const
    {
        return myViewer;
    }

    //! Return View.
    const Handle(V3d_View) & View() const
    {
        return myView;
    }

    //! Return AIS context.
    const Handle(AIS_InteractiveContext) & Context() const
    {
        return myContext;
    }

    //! Return OpenGL info.
    const QString &getGlInfo() const
    {
        return myGlInfo;
    }

    //! Minial widget size.
    QSize minimumSizeHint() const override
    {
        return QSize(200, 200);
    }

    //! Default widget size.
    QSize sizeHint() const override
    {
        return QSize(720, 480);
    }

public:
    //! Handle subview focus change.
    void OnSubviewChanged(const Handle(AIS_InteractiveContext) &, const Handle(V3d_View) &,
                          const Handle(V3d_View) & theNewView) override;

protected: // OpenGL events
    void initializeGL() override;
    void paintGL() override;
    // void resizeGL(int , int ) override;

protected: // user input events
    void closeEvent(QCloseEvent *theEvent) override;
    void keyPressEvent(QKeyEvent *theEvent) override;
    void mousePressEvent(QMouseEvent *theEvent) override;
    void mouseReleaseEvent(QMouseEvent *theEvent) override;
    void mouseMoveEvent(QMouseEvent *theEvent) override;
    void wheelEvent(QWheelEvent *theEvent) override;

private:
    //! Dump OpenGL info.
    void dumpGlInfo(bool theIsBasic, bool theToPrint);

    //! Request widget paintGL() event.
    void updateView();

    //! Handle view redraw.
    void handleViewRedraw(const Handle(AIS_InteractiveContext) & theCtx,
                          const Handle(V3d_View) & theView) override;

private:
    Handle(V3d_Viewer) myViewer;
    Handle(V3d_View) myView;
    Handle(AIS_InteractiveContext) myContext;
    Handle(AIS_ViewCube) myViewCube;

    Handle(V3d_View) myFocusView;

    QString myGlInfo;
    bool myIsCoreProfile;
};

#endif // OCCQT_VIEWER_H

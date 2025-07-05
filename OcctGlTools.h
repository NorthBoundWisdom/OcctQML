// Copyright (c) 2023 Kirill Gavrilov

#ifndef OCCQT_GL_TOOLS_H
#define OCCQT_GL_TOOLS_H

#include <QGuiApplication>
#include <QMouseEvent>

#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <Message.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Standard_WarningsDisable.hxx>
#include <Standard_WarningsRestore.hxx>
#include <V3d_View.hxx>

class OpenGl_Context;

//! Auxiliary wrapper to avoid OpenGL macros collisions between Qt and OCCT
//! headers.
class OcctGlTools
{
public:
    //! Return GL context.
    static Handle(OpenGl_Context) GetGlContext(const Handle(V3d_View) & theView);
    static Aspect_VKey qtKey2VKey(int theKey);
    static Aspect_VKeyFlags qtMouseModifiers2VKeys(Qt::KeyboardModifiers theModifiers);
    static Aspect_VKeyMouse qtMouseButtons2VKeys(Qt::MouseButtons theButtons);
};

#endif // _OcctGlTools_HeaderFile

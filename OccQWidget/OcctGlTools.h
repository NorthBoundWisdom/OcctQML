// Copyright (c) 2023 Kirill Gavrilov

#ifndef OCCQT_GL_TOOLS_H
#define OCCQT_GL_TOOLS_H

#include <QMessageBox>
#include <QMouseEvent>

#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_VKey.hxx>
#include <Aspect_VKeyFlags.hxx>
#include <OpenGl_Context.hxx>

class OpenGl_Context;

//! Auxiliary wrapper to avoid OpenGL macros collisions between Qt and OCCT headers.
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

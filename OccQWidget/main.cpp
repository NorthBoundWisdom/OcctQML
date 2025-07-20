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

#include <QApplication>
#include <QCoreApplication>

#include <Standard_WarningsDisable.hxx>
#include <Standard_WarningsRestore.hxx>

#include "MainWindow.h"

int main(int theNbArgs, char **theArgVec)
{
#if defined(_WIN32)
    // never use ANGLE on Windows, since OCCT 3D Viewer does not expect this
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    // QCoreApplication::setAttribute (Qt::AA_UseOpenGLES);
#elif defined(__APPLE__)
    qputenv("QT_QPA_PLATFORM", "cocoa");
#else
    // wayland-egl, minimal, xcb, vnc, linuxfb, eglfs, minimalegl, offscreen, wayland, vkkhrdisplay.
    qputenv("QT_QPA_PLATFORM", "xcb");

    // WSL2-specific workarounds
    const char *wslEnv = getenv("WSL_DISTRO_NAME");
    if (wslEnv != nullptr)
    {
        // Force software OpenGL in WSL2 if D3D12 causes issues
        // qputenv("LIBGL_ALWAYS_SOFTWARE", "1");

        // Disable VSync which can cause issues in WSL2
        qputenv("vblank_mode", "0");
        qputenv("__GL_SYNC_TO_VBLANK", "0");

        // Force OpenGL 3.3 compatibility profile for better WSL2 support
        qputenv("MESA_GL_VERSION_OVERRIDE", "3.3COMPAT");
        qputenv("MESA_GLSL_VERSION_OVERRIDE", "330");
    }
#endif

    QApplication aQApp(theNbArgs, theArgVec);

    MyMainWindow aMainWindow;
    aMainWindow.resize(aMainWindow.sizeHint());
    aMainWindow.show();
    return aQApp.exec();
}

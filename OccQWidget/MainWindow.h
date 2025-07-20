#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

//! Main application window.
class OcctQtViewer;
class MyMainWindow : public QMainWindow
{
    OcctQtViewer *myViewer;

public:
    MyMainWindow();

private:
    void setUpMainBar();
    void setUpViewer();
};

#endif // MAIN_WINDOW_H

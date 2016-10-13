#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>

using namespace cv;
using namespace std;

namespace Ui {
    class MainWindow;
}

using namespace cv;
using namespace std;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_actionSetImagesFolder_triggered();
    void on_actionSettings_triggered();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

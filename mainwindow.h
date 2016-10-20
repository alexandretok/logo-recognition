#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QFileDialog>
#include <QDebug>
#include <QString>
#include <QDir>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

#include <addlogo.h>
#include <settings.h>

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
    static const QString FOLDER_BRANDS;

private slots:

    void on_actionSetImagesFolder_triggered();
    void on_actionSettings_triggered();
    void on_actionRegisterLogo_triggered();

    void on_actionRun_triggered();

private:
    Ui::MainWindow *ui;
    AddLogo * addLogo;
    Settings * settings;
    bool match(QString scene);
    QString imagesFolder, brandsFolder;
    vector<KeyPoint> currentLogoKeypoints;
    Mat currentLogoDescriptors, currentLogo;
    void setCurrentLogoKeypoints();
    void setCurrentLogoDescriptors();
    void setCurrentLogo(QString logoPath);
};

#endif // MAINWINDOW_H

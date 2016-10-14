#ifndef ADDLOGO_H
#define ADDLOGO_H

#include <QDialog>
#include <QInputDialog>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QDir>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

namespace Ui {
class AddLogo;
}

class AddLogo : public QDialog
{
    Q_OBJECT

public:
    explicit AddLogo(QWidget *parent = 0);
    ~AddLogo();
    bool addLogo(int brandID, QString imgPath,QString outputFilePath);

private slots:
    void on_btCreateNew_clicked();
    void on_btAdd_clicked();
    void on_btSelectFiles_clicked();

private:
    Ui::AddLogo *ui;
    void closeEvent(QCloseEvent *);
    QStringList files;
};

#endif // ADDLOGO_H

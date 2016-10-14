#include "addlogo.h"
#include "ui_addlogo.h"

using namespace cv;
using namespace std;

AddLogo::AddLogo(QWidget *parent) : QDialog(parent), ui(new Ui::AddLogo) {
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    setFixedSize(this->geometry().size());
}

AddLogo::~AddLogo(){
    delete ui;
}

void AddLogo::closeEvent(QCloseEvent *){
    parentWidget()->setEnabled(true);
}

bool AddLogo::addLogo(int brandID, QString imgPath, QString outputFilePath){
    Mat img = imread(imgPath.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);

    Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();

    vector<KeyPoint> keypoints;
    f2d->detect(img, keypoints);

    Mat descriptors;
    f2d->compute(img, keypoints, descriptors);

    FileStorage fs(outputFilePath.toStdString(), FileStorage::WRITE);

    fs << "keypoints" << keypoints;
    fs << "descriptors" << descriptors;

    fs.release();

    return true;
}

void AddLogo::on_btCreateNew_clicked(){
    QString newBrand = QInputDialog::getText(this, "Creating new brand", "Name of the brand: ");
    qDebug() << newBrand;
}

void AddLogo::on_btAdd_clicked(){
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);

    int brandID = 1;

    /* Creating folders */
    QString brandFolder = "brands/" + QString::number(brandID);
    QString imagesFolder = brandFolder + "/images";
    if(!QDir(brandFolder).exists()){
        QDir().mkdir(brandFolder);
        QDir().mkdir(imagesFolder);
    }

    int i = 0;
    for (QStringList::iterator it = files.begin(); it != files.end(); ++it){
        /* Updates UI */
        qApp->processEvents();

        /* Gets current file path */
        QString current = *it;

        /* Gets file name */
        QStringList tmp = current.split("/");
        QString fileName = tmp.at(tmp.size()-1);

        /* Copies to set directory */
        QFile::copy(current, imagesFolder + "/" + fileName);

        addLogo(brandID, current, imagesFolder + "/" + fileName + ".yml");

        i++;
        ui->progressBar->setValue((int)(100 * i / files.size()));
    }

    QMessageBox::information(this, "Logos added", QString::number(i) + " logos added.");

    close();
}

void AddLogo::on_btSelectFiles_clicked(){
    files = QFileDialog::getOpenFileNames();
    ui->labelNumberOfSelectedFiles->setText(QString::number(files.size()) +  " files selected");
}

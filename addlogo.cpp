#include "addlogo.h"
#include "ui_addlogo.h"

#include "mainwindow.h"
#include <QMimeDatabase>
#include <QMimeType>

using namespace cv;
using namespace std;

#include <QDirIterator>

AddLogo::AddLogo(QWidget *parent) : QDialog(parent), ui(new Ui::AddLogo) {
    ui->setupUi(this);
    setFixedSize(this->geometry().size());

    updateBrands();
}

void AddLogo::updateBrands(){
    /* Clears combobox */
    ui->comboBrand->clear();

    ui->comboBrand->addItem("Select brand...");

    /* Get added brands from folders */
    QDirIterator brands(MainWindow::FOLDER_BRANDS, QDir::Dirs);
    while(brands.hasNext()){
        QString folder = brands.next().split(QDir::separator())[1];
        if(folder != ".." && folder != "."){
            folder[0] = folder[0].toUpper();
            ui->comboBrand->addItem(folder);
        }
    }
}

AddLogo::~AddLogo(){
    delete ui;
}

void AddLogo::on_btCreateNew_clicked(){
    QString newBrand = QInputDialog::getText(this, "Creating new brand", "Name of the brand: ");
    newBrand = newBrand.toLower();
    if(!QDir(MainWindow::FOLDER_BRANDS + QDir::separator() + newBrand).exists()){
        QDir().mkdir(MainWindow::FOLDER_BRANDS + QDir::separator() + newBrand);
    }

    updateBrands();

    newBrand[0] = newBrand[0].toUpper();
    ui->comboBrand->setCurrentText(newBrand);
}

void AddLogo::on_btAdd_clicked(){
    int added = 0;

    /* Which file types are accepted */
    QStringList validTypes(QList<QString>() << QString("image/jpeg") << QString("image/png"));

    QMimeDatabase db;

    if(ui->comboBrand->currentIndex() == 0)
        return;

    for (QStringList::iterator it = files.begin(); it != files.end(); ++it){
        /* Gets current file path */
        QString current = *it;

        /* Gets file name */
        QStringList tmp = current.split(QDir::separator());
        QString fileName = tmp.at(tmp.size()-1);

        QMimeType type = db.mimeTypeForFile(current);

        QString destination = MainWindow::FOLDER_BRANDS + QDir::separator() + ui->comboBrand->currentText().toLower() + QDir::separator() + fileName;

        /* Copies to set directory */
        if(validTypes.contains(type.name()) && QFile::copy(current, destination))
            added++;
    }

    QMessageBox::information(this, "Logos added", QString::number(added) + " logos added for " + ui->comboBrand->currentText() + ".");

    close();
}

void AddLogo::on_btSelectFiles_clicked(){
    files = QFileDialog::getOpenFileNames();
    ui->labelNumberOfSelectedFiles->setText(QString::number(files.size()) +  " files selected");
}

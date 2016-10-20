#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDirIterator>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>

const QString MainWindow::FOLDER_BRANDS = "brands";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    ui->progressBar->setVisible(false);
    ui->btCancel->setVisible(false);
    ui->tabs->clear();

    //-- Creates default folder
    if(!QDir(FOLDER_BRANDS).exists()){
        QDir().mkdir(FOLDER_BRANDS);
    }
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::setCurrentSceneKeypoints(){
    Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
    f2d->detect(currentScene, currentSceneKeypoints);
}

void MainWindow::setCurrentSceneDescriptors(){
    Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
    f2d->compute(currentScene, currentSceneKeypoints, currentSceneDescriptors);
}

void MainWindow::setCurrentScene(QString scenePath){
    currentScene = imread(scenePath.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
    setCurrentSceneKeypoints();
    setCurrentSceneDescriptors();
}


bool MainWindow::match(QString logoPath){
    bool match = true;

    Mat logo = imread(logoPath.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
//    Mat img_matches;

    Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
    //-- cv::Ptr<Feature2D> f2d = xfeatures2d::SURF::create();
    //-- cv::Ptr<Feature2D> f2d = ORB::create();

    //-- Step 1: Detect the keypoints:
    vector<KeyPoint> logoKeypoints;
    f2d->detect(logo, logoKeypoints);

    //-- Step 2: Calculate descriptors (feature vectors)
    Mat logoDescriptors;
    f2d->compute(logo, logoKeypoints, logoDescriptors);

    //-- Step 3: Matching descriptor vectors using BFMatcher :
    BFMatcher matcher;
    vector<DMatch> matches;
    matcher.match(logoDescriptors, currentSceneDescriptors, matches);

    double max_dist = 0; double min_dist = 10000;

    // Quick calculation of max and min distances between keypoints
    for( int i = 0; i < logoDescriptors.rows; i++ ){
        double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

//    qDebug() << "max:" << max_dist;
//    qDebug() << "min:" << min_dist;

    //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
    //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
    //-- small)
    //-- PS.- radiusMatch can also be used here.
    vector< DMatch > good_matches;

    for( int i = 0; i < logoDescriptors.rows; i++ ){
        if( matches[i].distance <= max(4*min_dist, 0.02)){
            good_matches.push_back( matches[i]);
        }
    }

//    drawMatches(obj, keypointsObject, scene, keypointsScene, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
//    imshow("img", img_matches);


      //-- Localize the object
      vector<Point2f> logoLocal;
      vector<Point2f> sceneLocal;

      for(int i = 0; i < good_matches.size(); i++){
        //-- Get the keypoints from the good matches
        logoLocal.push_back( logoKeypoints[ good_matches[i].queryIdx ].pt);
        sceneLocal.push_back( currentSceneKeypoints[ good_matches[i].trainIdx ].pt);
      }

      //-- This parameter needs to be adjusted
      if(good_matches.size() < 2){
          qDebug() << "Not enough good matches";
          return false;
      }

//      qDebug() << "good matches: " << good_matches.size();

      Mat H = findHomography(logoLocal, sceneLocal, RANSAC);

      //-- Get the corners from the image_1 ( the object to be "detected" )
      vector<Point2f> logoCorners(4);
      logoCorners[0] = cvPoint(0,0); logoCorners[1] = cvPoint( logo.cols, 0 );
      logoCorners[2] = cvPoint( logo.cols, logo.rows ); logoCorners[3] = cvPoint( 0, logo.rows );

      vector<Point2f> sceneCorners(4);
      sceneCorners[0] = cvPoint(0,0); sceneCorners[1] = cvPoint( currentScene.cols, 0 );
      sceneCorners[2] = cvPoint( currentScene.cols, currentScene.rows ); sceneCorners[3] = cvPoint( 0, currentScene.rows );

      vector<Point2f> outputCorners(4);

      perspectiveTransform(logoCorners, outputCorners, H);

      //-- I don't know if this should be considered a good test
      if(pointPolygonTest(sceneCorners, outputCorners[0], false) == -1 ||
         pointPolygonTest(sceneCorners, outputCorners[1], false) == -1 ||
         pointPolygonTest(sceneCorners, outputCorners[2], false) == -1 ||
         pointPolygonTest(sceneCorners, outputCorners[3], false) == -1)
          match = false;

      //-- This parameter needs to be adjusted
      if(contourArea(outputCorners) < 100)
          match = false;

      if(match){
          qDebug() << "Match";

          return true;
      } else {
          qDebug() << "No match";

          return false;
      }
}

void MainWindow::on_actionSetImagesFolder_triggered(){
    imagesFolder = QFileDialog::getExistingDirectory();
    ui->actionRun->setEnabled(true);
}

void MainWindow::on_actionSettings_triggered(){
    settings = new Settings(this);
    settings->show();
}

void MainWindow::on_actionRegisterLogo_triggered(){
    addLogo = new AddLogo(this);
    addLogo->show();
}

void MainWindow::on_actionRun_triggered(){
    //-- Shows progress bar and cancel button
    ui->progressBar->setVisible(true);
    ui->btCancel->setVisible(true);

    ui->progressBar->setValue(0);
    ui->tabs->clear();

    QDirIterator * brands;
    QDirIterator * scenes;

    //-- Counting how many images are going to be processed (to update the progressbar accordingly)
    brands = new QDirIterator(FOLDER_BRANDS);
    int totalImages = 0,
        totalLogos  = 0,
        totalScenes = QDir(imagesFolder).entryList(QStringList() << "*.jpg" << "*.png").size();
    while(brands->hasNext()){
        QString b = brands->next();
        if(b.indexOf(QDir::separator() + QString(".")) == -1){
            totalLogos += QDir(b).entryList(QStringList() << "*.jpg" << "*.png").size();
        }
    }
    totalImages = totalLogos * totalScenes;

    int imagesProcessed = 0;
    QMap<QString, int> tabIndex; //-- Keeps track of which tab belong to which brand
    QMap<QString, int> imagesMatched;


    //-- In the following nested loops, we iterate through all the scenes and try to find
    //-- each one of the brands' logos on them
    scenes = new QDirIterator(imagesFolder, QStringList() << "*.jpg" << "*.png");
    while(scenes->hasNext()){
        QString scene = scenes->next();

        //-- Here, we compute the keypoints and descriptors for the current scene
        //-- This makes the program run faster, because we only compute these once
        setCurrentScene(scene);

        brands = new QDirIterator(FOLDER_BRANDS);

        while(brands->hasNext()){
            QString brand = brands->next();

            //-- This if skips the "." and ".." directories
            if(brand.indexOf(QDir::separator() + QString(".")) == -1){
                imagesMatched[brand] = 0;
                QDirIterator logos(brand, QStringList() << "*.jpg" << "*.png");

                brand = brand.split(QDir::separator())[1];
                brand[0] = brand[0].toUpper();

                //-- Verifies if the current brand already has a tab associated
                QGridLayout * layout;
                if(!tabIndex.contains(brand)){
                    //-- Creates a new tab for the current brand
                    layout = new QGridLayout;
                    ui->tabs->addTab(new QWidget(), brand);

                    //-- Saves the tab index for later use
                    tabIndex[brand] = ui->tabs->count() - 1;
                    ui->tabs->widget(tabIndex[brand])->setLayout(layout);

                    layout->setColumnStretch(0, 1);
                    layout->setColumnStretch(1, 1);
                    layout->setColumnStretch(2, 1);
                    layout->setColumnStretch(3, 1);
                    layout->setRowStretch(0, 1);
                    layout->setRowStretch(1, 1);
                    layout->setRowStretch(2, 1);
                    layout->setRowStretch(3, 1);
                    layout->setRowStretch(4, 1);
                    layout->setRowStretch(5, 1);
                } else {
                    //-- If the brands already has a tab associated, just get the layout instance
                    layout = (QGridLayout*) ui->tabs->widget(tabIndex[brand])->layout();
                }

                while(logos.hasNext()){
                    QString logo = logos.next();

                    if(match(logo)){
                        //-- If the logo was found in the scene
                        QLabel * label = new QLabel();
                        QPixmap pixmap(scene);
                        pixmap = pixmap.scaled(128, 128);
                        label->setStyleSheet("border: 2px solid grey;");
                        label->setPixmap(pixmap);
                        label->show();
                        layout->addWidget(label, imagesMatched[brand] / 4, imagesMatched[brand] % 4);
                        imagesMatched[brand]++;
                        ui->tabs->setTabText(tabIndex[brand], brand + " (" + QString::number(imagesMatched[brand]) + ")");
                    }
                    imagesProcessed++;
                    ui->progressBar->setValue(100 * imagesProcessed / totalImages);
                    qApp->processEvents(); //-- Allows the application to update the GUI
                }
            }
        }
    }
    ui->btCancel->setVisible(false);
}

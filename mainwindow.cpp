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
    ui->tabs->clear();

    /* Creating default folders */
    if(!QDir(FOLDER_BRANDS).exists()){
        QDir().mkdir(FOLDER_BRANDS);
    }

//    match("/home/alexandre/projetos/logo-recognition/test-images/sony.jpg", "/home/alexandre/projetos/logo-recognition/test-images/camera.jpg");
//    waitKey();

//    match("/home/alexandre/projetos/logo-recognition/test-images/sony.jpg", "/home/alexandre/projetos/logo-recognition/test-images/head.jpg");
//    waitKey();

//    match("/home/alexandre/projetos/logo-recognition/test-images/sony.jpg", "/home/alexandre/projetos/logo-recognition/test-images/sony1.jpg");
//    waitKey();

//    match("/home/alexandre/projetos/logo-recognition/test-images/sony.jpg", "/home/alexandre/projetos/logo-recognition/test-images/sony2.jpg");
//    waitKey();
}

MainWindow::~MainWindow(){
    delete ui;
}


bool MainWindow::match(QString objPath, QString scenePath){
    qDebug() << "processando: " << scenePath;
//    qDebug() << "logo: " << objPath;

    bool match = true;

    Mat obj = imread(objPath.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
    Mat scene = imread(scenePath.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
    Mat img_matches;
//    namedWindow("img", WINDOW_NORMAL);

    Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
    //-- cv::Ptr<Feature2D> f2d = xfeatures2d::SURF::create();
    //-- cv::Ptr<Feature2D> f2d = ORB::create();

    //-- Step 1: Detect the keypoints:
    vector<KeyPoint> keypointsObject, keypointsScene;
    f2d->detect(obj, keypointsObject);
    f2d->detect(scene, keypointsScene);

    //-- Step 2: Calculate descriptors (feature vectors)
    Mat descriptorsObj, descriptorsScene;
    f2d->compute(obj, keypointsObject, descriptorsObj );
    f2d->compute(scene, keypointsScene, descriptorsScene );

    //-- Step 3: Matching descriptor vectors using BFMatcher :
    BFMatcher matcher;
    vector<DMatch> matches;
    matcher.match(descriptorsObj, descriptorsScene, matches);

    double max_dist = 0; double min_dist = 10000;

    // Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptorsObj.rows; i++ ){
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

    for( int i = 0; i < descriptorsObj.rows; i++ ){
        if( matches[i].distance <= max(4*min_dist, 0.02)){
            good_matches.push_back( matches[i]);
        }
    }

//    drawMatches(obj, keypointsObject, scene, keypointsScene, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
//    imshow("img", img_matches);


    //-- Localize the object
      vector<Point2f> objLoc;
      vector<Point2f> sceneLoc;

      for(int i = 0; i < good_matches.size(); i++ ){
        //-- Get the keypoints from the good matches
        objLoc.push_back( keypointsObject[ good_matches[i].queryIdx ].pt );
        sceneLoc.push_back( keypointsScene[ good_matches[i].trainIdx ].pt );
      }

      if(good_matches.size() < 2){
          qDebug() << "nenhum match bom";
          return false;
      }

//      qDebug() << "good matches: " << good_matches.size();

      Mat H = findHomography(objLoc, sceneLoc, RANSAC );

      //-- Get the corners from the image_1 ( the object to be "detected" )
      vector<Point2f> obj_corners(4);
      obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( obj.cols, 0 );
      obj_corners[2] = cvPoint( obj.cols, obj.rows ); obj_corners[3] = cvPoint( 0, obj.rows );

      vector<Point2f> scene_corners(4);
      scene_corners[0] = cvPoint(0,0); scene_corners[1] = cvPoint( scene.cols, 0 );
      scene_corners[2] = cvPoint( scene.cols, scene.rows ); scene_corners[3] = cvPoint( 0, scene.rows );

      vector<Point2f> logo_corners(4);

        perspectiveTransform(obj_corners, logo_corners, H);

//      qDebug() << "OBJ CORNERS: ";

//      qDebug() << obj_corners[0].x << obj_corners[0].y;
//      qDebug() << obj_corners[1].x << obj_corners[1].y;
//      qDebug() << obj_corners[2].x << obj_corners[2].y;
//      qDebug() << obj_corners[3].x << obj_corners[3].y;

//      qDebug() << "SCENE CORNERS: ";

//      qDebug() << logo_corners[0].x << logo_corners[0].y;
//      qDebug() << logo_corners[1].x << logo_corners[1].y;
//      qDebug() << logo_corners[2].x << logo_corners[2].y;
//      qDebug() << logo_corners[3].x << logo_corners[3].y;

//      qDebug() << "POINT TEST: ";

//      qDebug() << pointPolygonTest(scene_corners, logo_corners[0], false)
//               << pointPolygonTest(scene_corners, logo_corners[1], false)
//               << pointPolygonTest(scene_corners, logo_corners[2], false)
//               << pointPolygonTest(scene_corners, logo_corners[3], false);

      if(pointPolygonTest(scene_corners, logo_corners[0], false) == -1 ||
         pointPolygonTest(scene_corners, logo_corners[1], false) == -1 ||
         pointPolygonTest(scene_corners, logo_corners[2], false) == -1 ||
         pointPolygonTest(scene_corners, logo_corners[3], false) == -1)
          match = false;

//      qDebug() << "AREA: " << contourArea(logo_corners);

      if(contourArea(logo_corners) < 100)
          match = false;

      Mat tmp = imread(scenePath.toStdString(), CV_LOAD_IMAGE_COLOR);

      if(match){
//          qDebug() << "MATCH";

          //-- Draw lines between the corners (the mapped object in the scene - image_2 )
          line(tmp, logo_corners[0], logo_corners[1], Scalar(0, 255, 0), 4);
          line(tmp, logo_corners[1], logo_corners[2], Scalar( 0, 255, 0), 4);;
          line(tmp, logo_corners[2], logo_corners[3], Scalar( 0, 255, 0), 4);
          line(tmp, logo_corners[3], logo_corners[0], Scalar( 0, 255, 0), 4);

          return true;
      } else {
//          qDebug() << "NOT MATCH";

          return false;
      }

      //-- Show detected matches
//      imshow("img", tmp);
//      waitKey();

      return true;
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
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);
    ui->tabs->clear();
    qApp->processEvents();

    QDirIterator * brands = new QDirIterator(FOLDER_BRANDS);
    QDirIterator * images;

    /* Counting how many images are going to be processed for the progressbar */
    int totalImages = 0;
    int imgCount = QDir(imagesFolder).entryList().size() - 2;
    while(brands->hasNext()){
        QString b = brands->next();
        if(b.indexOf(QDir::separator() + QString(".")) == -1){
            totalImages += QDir(b).entryList().size() - 2;
        }
    }
    totalImages *= imgCount;

    int imagesProcessed = 0;
    brands = new QDirIterator(FOLDER_BRANDS);
    int tabIndex = -1;

    /* These loops try to match all the images with each logo from each brand */
    while(brands->hasNext()){
        int imagesMatched = 0;
        QString brand = brands->next();

        if(brand.indexOf(QDir::separator() + QString(".")) == -1){
            QDirIterator logos(brand, QStringList() << "*.jpg" << "*.png");

            /* Create new tab with brand name */
            brand = brand.split(QDir::separator())[1];
            brand[0] = brand[0].toUpper();

            QGridLayout * layout = new QGridLayout;
            tabIndex++;
            ui->tabs->addTab(new QWidget(), brand);
            ui->tabs->setCurrentIndex(ui->tabs->count() - 1);
            ui->tabs->currentWidget()->setLayout(layout);
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

            while(logos.hasNext()){
                QString logo = logos.next();
                images = new QDirIterator(imagesFolder, QStringList() << "*.jpg" << "*.png");
                int i=0;
                while(images->hasNext()){
                    QString image = images->next();
                    if(match(logo, image)){
                        QLabel * label = new QLabel();
                        QPixmap pixmap(image);
                        pixmap = pixmap.scaled(128, 128);
                        label->setStyleSheet("border: 2px solid grey;");
                        label->setPixmap(pixmap);
                        label->show();
                        layout->addWidget(label, imagesMatched / 4, imagesMatched % 4);
                        imagesMatched++;
                        if(imagesMatched > 0)
                            ui->tabs->setTabText(tabIndex, brand + " (" + QString::number(imagesMatched) + ")");
                    }
                    imagesProcessed++;
//                    qDebug() << "processed: " << imagesProcessed;
//                    qDebug() << "total: " << totalImages;
                    ui->progressBar->setValue(100 * imagesProcessed / totalImages);
                    qApp->processEvents();
                }
            }

            layout->addWidget(new QPushButton("Exportar"), 1 + imagesMatched / 4, 3);
        }
    }

    ui->tabs->setCurrentIndex(0);
}

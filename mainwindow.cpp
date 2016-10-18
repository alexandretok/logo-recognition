#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDirIterator>

const QString MainWindow::FOLDER_BRANDS = "brands";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

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

bool MainWindow::match(QString obj, QString scene){
    return match(imread(obj.toStdString(), CV_LOAD_IMAGE_GRAYSCALE), imread(scene.toStdString(), CV_LOAD_IMAGE_GRAYSCALE));
}

bool MainWindow::match(Mat obj, Mat scene){
    Mat img_matches;
    namedWindow("img", WINDOW_NORMAL);

    Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
    //-- cv::Ptr<Feature2D> f2d = xfeatures2d::SURF::create();
    //-- cv::Ptr<Feature2D> f2d = ORB::create();

    //-- Step 1: Detect the keypoints:
    vector<KeyPoint> keypointsObject, keypointsScene;
    f2d->detect(obj, keypointsObject);
    f2d->detect(scene, keypointsScene);

    qDebug() << "passou detect";

    //-- Step 2: Calculate descriptors (feature vectors)
    Mat descriptorsObj, descriptorsScene;
    f2d->compute(obj, keypointsObject, descriptorsObj );
    f2d->compute(scene, keypointsScene, descriptorsScene );

    qDebug() << "passou compute";
    qDebug() << "keypoints: " << keypointsScene.size();

    //-- Step 3: Matching descriptor vectors using BFMatcher :
    BFMatcher matcher;
    vector<DMatch> matches;
    matcher.match(descriptorsObj, descriptorsScene, matches);

    qDebug() << "passou match";

    double max_dist = 0; double min_dist = 100;

    // Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptorsObj.rows; i++ ){
        double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

    qDebug() << "max:" << max_dist;
    qDebug() << "min:" << min_dist;

    //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
    //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
    //-- small)
    //-- PS.- radiusMatch can also be used here.
    vector< DMatch > good_matches;

    for( int i = 0; i < descriptorsObj.rows; i++ ){
        if( matches[i].distance <= max(3*min_dist, 0.02)){
            good_matches.push_back( matches[i]);
        }
    }

    drawMatches(obj, keypointsObject, scene, keypointsScene, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    imshow("img", img_matches);


    //-- Localize the object
      vector<Point2f> objLoc;
      vector<Point2f> sceneLoc;

      for( int i = 0; i < good_matches.size(); i++ ){
        //-- Get the keypoints from the good matches
        objLoc.push_back( keypointsObject[ good_matches[i].queryIdx ].pt );
        sceneLoc.push_back( keypointsScene[ good_matches[i].trainIdx ].pt );
      }

      qDebug() << good_matches.size();
      qDebug() << objLoc.size();
      qDebug() << sceneLoc.size();

      Mat H = findHomography( objLoc, sceneLoc, RANSAC );

      //-- Get the corners from the image_1 ( the object to be "detected" )
      vector<Point2f> obj_corners(4);
      obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( obj.cols, 0 );
      obj_corners[2] = cvPoint( obj.cols, obj.rows ); obj_corners[3] = cvPoint( 0, obj.rows );
      vector<Point2f> scene_corners(4);

      perspectiveTransform(obj_corners, scene_corners, H);

      //-- Draw lines between the corners (the mapped object in the scene - image_2 )
      line( img_matches, scene_corners[0] + Point2f( obj.cols, 0), scene_corners[1] + Point2f( obj.cols, 0), Scalar(0, 255, 0), 4 );
      line( img_matches, scene_corners[1] + Point2f( obj.cols, 0), scene_corners[2] + Point2f( obj.cols, 0), Scalar( 0, 255, 0), 4 );
      line( img_matches, scene_corners[2] + Point2f( obj.cols, 0), scene_corners[3] + Point2f( obj.cols, 0), Scalar( 0, 255, 0), 4 );
      line( img_matches, scene_corners[3] + Point2f( obj.cols, 0), scene_corners[0] + Point2f( obj.cols, 0), Scalar( 0, 255, 0), 4 );

      //-- Show detected matches
      imshow("img", img_matches);

      return true;
}

void MainWindow::on_actionSetImagesFolder_triggered(){
    imagesFolder = QFileDialog::getExistingDirectory();
    qDebug() << imagesFolder;
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
    QDirIterator logos(FOLDER_BRANDS);
    QDirIterator images(imagesFolder, QStringList() << "*.jpg" << "*.png");
    while(logos.hasNext()){
        qDebug() << logos.next();

        while(images.hasNext()){
            qDebug() << ("\t" + images.next());
        }
    }
}

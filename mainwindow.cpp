#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    /* Creating default folders */
    if(!QDir("brands").exists()){
        QDir().mkdir("brands");
    }

    namedWindow("img", WINDOW_NORMAL);

    Mat img_1 = imread("/home/alexandre/Pictures/sony.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    Mat img_2 = imread("/home/alexandre/Pictures/sony5.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    Mat img_matches;

    //
    // now, you can no more create an instance on the 'stack', like in the tutorial
    // (yea, noticed for a fix/pr).
    // you will have to use cv::Ptr all the way down:
    //
    Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
    //cv::Ptr<Feature2D> f2d = xfeatures2d::SURF::create();
    //cv::Ptr<Feature2D> f2d = ORB::create();
    // you get the picture, i hope..

    //-- Step 1: Detect the keypoints:
    vector<KeyPoint> keypoints_1, keypoints_2;
    f2d->detect(img_1, keypoints_1);
    f2d->detect(img_2, keypoints_2);

    //-- Step 2: Calculate descriptors (feature vectors)
    Mat descriptors_1, descriptors_2;
    f2d->compute( img_1, keypoints_1, descriptors_1 );
//    drawKeypoints(img_1, keypoints_1, img_1);
//    imshow("img", img_1);

    f2d->compute(img_2, keypoints_2, descriptors_2 );

    //-- Step 3: Matching descriptor vectors using BFMatcher :
    BFMatcher matcher;
    vector<DMatch> matches;
    matcher.match(descriptors_1, descriptors_2, matches);

    double max_dist = 0; double min_dist = 100;

    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_1.rows; i++ ){
        double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

    qDebug() << "max" << max_dist;
    qDebug() << "min" << min_dist;

    //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
    //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
    //-- small)
    //-- PS.- radiusMatch can also be used here.
    std::vector< DMatch > good_matches;

    for( int i = 0; i < descriptors_1.rows; i++ ){
        if( matches[i].distance <= 3*min_dist){
            good_matches.push_back( matches[i]);
        }
    }

    drawMatches(img_1, keypoints_1, img_2, keypoints_2, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    imshow("img", img_matches);


    //-- Localize the object
      std::vector<Point2f> obj;
      std::vector<Point2f> scene;

      for( int i = 0; i < good_matches.size(); i++ )
      {
        //-- Get the keypoints from the good matches
        obj.push_back( keypoints_1[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_2[ good_matches[i].trainIdx ].pt );
      }

      qDebug() << good_matches.size();

      Mat H = findHomography( obj, scene, RANSAC );

      //-- Get the corners from the image_1 ( the object to be "detected" )
      std::vector<Point2f> obj_corners(4);
      obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( img_1.cols, 0 );
      obj_corners[2] = cvPoint( img_1.cols, img_1.rows ); obj_corners[3] = cvPoint( 0, img_1.rows );
      std::vector<Point2f> scene_corners(4);

      perspectiveTransform( obj_corners, scene_corners, H);

      //-- Draw lines between the corners (the mapped object in the scene - image_2 )
      line( img_matches, scene_corners[0] + Point2f( img_1.cols, 0), scene_corners[1] + Point2f( img_1.cols, 0), Scalar(0, 255, 0), 4 );
      line( img_matches, scene_corners[1] + Point2f( img_1.cols, 0), scene_corners[2] + Point2f( img_1.cols, 0), Scalar( 0, 255, 0), 4 );
      line( img_matches, scene_corners[2] + Point2f( img_1.cols, 0), scene_corners[3] + Point2f( img_1.cols, 0), Scalar( 0, 255, 0), 4 );
      line( img_matches, scene_corners[3] + Point2f( img_1.cols, 0), scene_corners[0] + Point2f( img_1.cols, 0), Scalar( 0, 255, 0), 4 );

      //-- Show detected matches
      imshow( "img", img_matches );
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::on_actionSetImagesFolder_triggered(){
    QString folder = QFileDialog::getExistingDirectory();
    qDebug() << folder;
}

void MainWindow::on_actionSettings_triggered(){
    settings = new Settings(this);
    settings->show();
}

void MainWindow::on_actionRegisterLogo_triggered(){
    addLogo = new AddLogo(this);
    addLogo->show();
}

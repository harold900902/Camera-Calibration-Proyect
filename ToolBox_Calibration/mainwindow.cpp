#include "mainwindow.h"
#include "ui_mainwindow.h"


const Scalar RED(0,0,255), GREEN(0,255,0);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::clearLayout(){
    if (ui->horizontalLayout->layout()!= NULL) {
        QLayoutItem* item;
            while ( ( item = ui->horizontalLayout->layout()->takeAt(0) ) != NULL )
            {
                delete item->widget();
                delete item;
            }
    }
}
QPixmap MainWindow::Mat2QPixmap(const cv::Mat &mat)
{
    cv::Mat rgb;
    QPixmap p;
    cvtColor(mat, rgb, CV_BGR2RGB);
    p.convertFromImage(QImage(static_cast<const unsigned char*>(rgb.data), rgb.cols, rgb.rows, QImage::Format_RGB888));
    return p;
}
void MainWindow::DetectionImage(){
    QPixmap pixmap;
    //Mat image;
    int pos=0;
    QLabel *labels[s.nrFrames];
    imagePoints.clear();
    clearLayout();
    if (s.inputType == Setting::IMAGE_LIST) {
        mode=Setting::CAPTURING;

    }else {
        mode= Setting::DETECTION;
    }

    if (s.inputType == Setting::CAMERA)
      s.inputCapture.open(s.cameraID);
    else
      s.inputCapture.release();

    for(int i = 0;;++i)
    {
      Mat view;
      bool blinkOutput = false;

      view = s.nextImage().clone();


       if( mode == Setting::CAPTURING && imagePoints.size() >= (unsigned)s.nrFrames ){
          if (s.inputType == Setting::CAMERA)
            s.inputCapture.release();
          break;
       }

        imageSize = view.size();  // Format input image.
        if( s.flipVertical )    flip( view, view, 0 );

        vector<Point2f> pointBuf;

        bool found;
        switch( s.calibrationPattern ) // Find feature points on the input format
        {
        case Setting::CHESSBOARD:
            found = findChessboardCorners( view, s.boardSize, pointBuf,
                CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
            break;
        case Setting::CIRCLES_GRID:
            found = findCirclesGrid( view, s.boardSize, pointBuf );
            break;
        case Setting::ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid( view, s.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID );
            break;
        default:
            found = false;
            break;
        }

        if ( found)                // If done with success,
        {
              // improve the found corners' coordinate accuracy for chessboard
                if( s.calibrationPattern == Setting::CHESSBOARD)
                {
                    Mat viewGray;
                    cvtColor(view, viewGray, COLOR_BGR2GRAY);
                    cornerSubPix( viewGray, pointBuf, Size(11,11),
                        Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
                }
                // Draw the corners.
                drawChessboardCorners( view, s.boardSize, Mat(pointBuf), found );
                if( mode == Setting::CAPTURING &&  // For camera only take new samples after delay time
                    (!s.inputCapture.isOpened() || clock() - prevTimestamp > s.delay*1e-3*CLOCKS_PER_SEC) )
                {
                    imagePoints.push_back(pointBuf);
                    prevTimestamp = clock();
                    blinkOutput = s.inputCapture.isOpened();

                    pixmap= Mat2QPixmap(view.clone()).scaledToWidth(100);
                    labels[pos] = new QLabel();
                    labels[pos]->setPixmap(pixmap);
                    ui->horizontalLayout->addWidget(labels[pos++]);
                }




        }
        //----------------------------- Output Text ------------------------------------------------
               string msg = (mode == Setting::CAPTURING) ? "100/100" :
                             mode == Setting::CALIBRATED ? "Calibrated" : "Press 'g' to start";
               int baseLine = 0;
               Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
               Point textOrigin(view.cols - 2*textSize.width - 10, view.rows - 2*baseLine - 10);

               if( mode == Setting::CAPTURING )
               {
                   if(s.showUndistorsed)
                       msg = format( "%d/%d Undist", (int)imagePoints.size(), s.nrFrames );
                   else
                       msg = format( "%d/%d", (int)imagePoints.size(), s.nrFrames );
               }

               putText( view, msg, textOrigin, 1, 1, mode == Setting::CALIBRATED ?  GREEN : RED);

               if( blinkOutput )
                   bitwise_not(view, view);

               //------------------------------ Show image and check for input commands -------------------
               imshow("Image View", view);
               char key = (char)waitKey(s.inputCapture.isOpened() ? 50 : s.delay);

               if( key  == ESC_KEY )
                   break;

               if( key == 'u' && mode == Setting::CALIBRATED )
                  s.showUndistorsed = !s.showUndistorsed;

               if( s.inputCapture.isOpened() && key == 'g' )
               {
                   mode = Setting::CAPTURING;
                   imagePoints.clear();
               }
           }
}
void MainWindow::on_actionLoad_XML_triggered()
{

      ui->listWidget->clear();
      QString filename = QFileDialog::getOpenFileName(0, "Load XML", "", "File (*.xml)");
      const string inputSettingsFile= filename.toStdString();
      FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
      if (!fs.isOpened())
      {
          QListWidgetItem* elemento;
          elemento = new QListWidgetItem(trUtf8("Could not open the configuration file: "));
          ui->listWidget->addItem(elemento);
      }else{
      s.read(fs["Settings"]);
      fs.release();      // close Settings file
      QListWidgetItem* elemento;
      elemento = new QListWidgetItem(trUtf8("File successfully uploaded "));
      ui->listWidget->addItem(elemento);
        }
      if (!s.goodInput)
      {
          QListWidgetItem* elemento;
          elemento = new QListWidgetItem(trUtf8("Invalid input detected. Application stopping. "));
          ui->listWidget->addItem(elemento);

      }
}


void MainWindow::on_actionClose_triggered()
{
    this->close();
}

void MainWindow::on_actionCapture_triggered()
{
       s.imageList.clear();
       s.interprate();
       DetectionImage();
       QListWidgetItem* elemento;
       elemento = new QListWidgetItem(trUtf8("Images captured successfully. "));
       ui->listWidget->addItem(elemento);
}

void MainWindow::on_actionShow_undistorted_image_triggered()
{
    // -----------------------Show the undistorted image for the image list ------------------------
    if( s.inputType == Setting::IMAGE_LIST && s.showUndistorsed )
    {
        Mat view, rview, map1, map2;
           initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
            getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
            imageSize, CV_16SC2, map1, map2);

        for(int i = 0; i < (int)s.imageList.size(); i++ )
        {
            view = imread(s.imageList[i], 1);
            if(view.empty())
                continue;
            remap(view, rview, map1, map2, INTER_LINEAR);


            imshow("Image View", rview);
            char c = (char)waitKey();
            if( c  == ESC_KEY)
                break;
        }
    }
}

void MainWindow::on_actionCalibrate_triggered()
{
    //-----  If no more image, or got enough, then stop calibration and show result -------------
    if( mode == Setting::CAPTURING && imagePoints.size() >= (unsigned)s.nrFrames )
    {

        ok = s.runCalibration(s,imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs,
                                 reprojErrs, totalAvgErr);
        if (ok) {
            mode = Setting::CALIBRATED;
            QListWidgetItem* elemento;
            elemento = new QListWidgetItem(trUtf8("Calibration succeeded. "));
            ui->listWidget->addItem(elemento);
          /*  elemento = new QListWidgetItem(trUtf8(". avg re projection error = "  + totalAvgErr));
            ui->listWidget->addItem(elemento);*/
        }else {
            mode = Setting::DETECTION;
            QListWidgetItem* elemento;
            elemento = new QListWidgetItem(trUtf8("Calibration failed. "));
            ui->listWidget->addItem(elemento);
        }


     }

}

void MainWindow::on_actionShow_Calibrated_Camera_triggered()
{
    //------------------------- Video capture  output  undistorted ------------------------------
    Mat image, imageUndist;
    char key;
    if (s.inputType == Setting::CAMERA){
      s.inputCapture.open(s.cameraID);

     while (1)
     {
       image= s.nextImage();
       undistort(image, imageUndist, cameraMatrix, distCoeffs);
       imshow("Calibrated camera", imageUndist);
        key = (char)waitKey();
        if (key == ESC_KEY )
        {
            s.inputCapture.release();
          break;
        }
     }
    }else if (s.inputType == Setting::IMAGE_LIST) {
        for(int i = 0; i < (int)s.imageList.size(); i++ )
        {
            image = imread(s.imageList[i], 1);
            if(image.empty())
                continue;
            undistort(image, imageUndist, cameraMatrix, distCoeffs);
            imshow("Image View", image);
            key =  (char)waitKey();
            if (key == ESC_KEY )
            {
             break;
            }
        }
    }

}

void MainWindow::on_actionLoad_triggered()
{

    s.imageList.clear();
    QStringList filenames = QFileDialog::getOpenFileNames(0, "Load images", "", "Images (*.jpg;*.jpeg;*.bmp;*.png)");
    QListWidgetItem* elemento;

    if (!filenames.isEmpty()) {
        for (int i=0;i<filenames.length();++i)
         {
          s.imageList.push_back(filenames.at(i).toStdString());
          ui->listWidget->clear();
          QListWidgetItem* elemento;
          elemento = new QListWidgetItem(trUtf8("Load images... "));
          ui->listWidget->addItem(elemento);
         }

        s.interprate();
        DetectionImage();

        elemento = new QListWidgetItem(trUtf8("Load images. Done!!! "));
        ui->listWidget->addItem(elemento);
    }else {

        elemento = new QListWidgetItem(trUtf8("Load images. Failed!!! "));
        ui->listWidget->addItem(elemento);
    }

}

void MainWindow::on_actionSafe_YML_triggered()
{
   QString dir = QFileDialog::getExistingDirectory(this, tr("Save XML"),
                                                 "/C:",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    QListWidgetItem* elemento;
    if (ok && !dir.isEmpty()) {
        s.outputFileName = dir.toStdString()+"/"+s.outputFileName;
        s.saveCameraParams(s, imageSize, cameraMatrix, distCoeffs, rvecs ,tvecs, reprojErrs,
                            imagePoints, totalAvgErr);

        elemento = new QListWidgetItem(trUtf8("Parameters successfully saved. "));
        ui->listWidget->addItem(elemento);
    }else {
        elemento = new QListWidgetItem(trUtf8("An error occurred while attempting to save parameters. "));
        ui->listWidget->addItem(elemento);
    }

}

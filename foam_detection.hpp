#ifndef FOAM_DETECTION_HPP
#define FOAM_DETECTION_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <fstream>
#include <chrono>
#include <iostream>

using namespace cv;
using namespace std;

class FoamDetector {
public:
    FoamDetector();
    
    int getAlertThreshold() const;
    void setAlertThreshold(int threshold);
    void setPlaybackSpeed(double speed);
    
    void processVideo(int camera_index);
    void processVideo(const string& video_path); // 移除了多余的类名限定符
    
public:
    Mat preprocessFrame(Mat frame);
    Mat detectEdges(Mat frame);
    Mat detectDynamicChanges(Mat prev_frame, Mat current_frame);
    vector<Rect> smartMorphology(Mat binary_img);
    int processFrame(Mat frame, Mat& prev_frame, Mat& result_frame);
    
    int alert_threshold;
    double playback_speed;
};

#endif // FOAM_DETECTION_HPP

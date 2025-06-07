#include "foam_detection.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

FoamDetector::FoamDetector() : alert_threshold(100), playback_speed(1.0) {}

int FoamDetector::getAlertThreshold() const {
	return alert_threshold;
}

void FoamDetector::setAlertThreshold(int threshold) {
    alert_threshold = threshold;
}

void FoamDetector::setPlaybackSpeed(double speed) {
    playback_speed = speed;
}

// 重载processVideo函数以支持视频文件路径
void FoamDetector::processVideo(const string& video_path) {
    VideoCapture cap(video_path);
    
    if (!cap.isOpened()) {
        cerr << "无法打开视频文件: " << video_path << endl;
        return;
    }
    
    double fps = cap.get(CAP_PROP_FPS);
    Mat prev_frame;

    while (true) {
        Mat frame;
        if (!cap.read(frame)) break;

        Mat result_frame;
        int foam_count = processFrame(frame, prev_frame, result_frame);

        imshow("Foam Detection", result_frame);
        prev_frame = preprocessFrame(frame);

        if (waitKey(static_cast<int>(1000 / (fps * playback_speed))) == 27) break;
    }

    cap.release();
}

void FoamDetector::processVideo(int camera_index) {
    VideoCapture cap(camera_index);
    
    if (!cap.isOpened()) {
        cerr << "无法打开摄像头" << endl;
        return;
    }
    
    double fps = cap.get(CAP_PROP_FPS);
    Mat prev_frame;

    while (true) {
        Mat frame;
        if (!cap.read(frame)) break;

        Mat result_frame;
        int foam_count = processFrame(frame, prev_frame, result_frame);

        imshow("Foam Detection", result_frame);
        prev_frame = preprocessFrame(frame);

        if (waitKey(static_cast<int>(1000 / (fps * playback_speed))) == 27) break;
    }

    cap.release();
    //return int foam_count;
}

Mat FoamDetector::preprocessFrame(Mat frame) {
    // 实现预处理逻辑
		Mat resized_frame;
        resize(frame, resized_frame, Size(frame.cols / 2, frame.rows / 2));

        Mat hsv;
        cvtColor(resized_frame, hsv, COLOR_BGR2HSV);

        vector<Mat> hsv_channels;
        split(hsv, hsv_channels);
        Mat v_channel = hsv_channels[2];

        Mat blurred;
        bilateralFilter(v_channel, blurred, 5, 50, 50);

        Mat blurred_3ch;
        cvtColor(blurred, blurred_3ch, COLOR_GRAY2BGR);

        Mat enhanced;
        detailEnhance(blurred_3ch, enhanced, 10, 0.15);

        Mat enhanced_gray;
        cvtColor(enhanced, enhanced_gray, COLOR_BGR2GRAY);

        Ptr<CLAHE> clahe = createCLAHE(3.0, Size(16, 16));
        Mat clahe_img;
        clahe->apply(enhanced_gray, clahe_img);

        return clahe_img;
}

Mat FoamDetector::detectEdges(Mat frame) {
    vector<double> scales = { 1.0, 1.5, 2.0 }; // 或显式构造：vector<double>{...}
    int ksize = 3;
    Mat edges = Mat::zeros(frame.size(), CV_8UC1);

    Mat sobelX_total, sobelY_total; // 确保初始化
    for (double scale : scales) {
        Mat sobelX, sobelY;
        Sobel(frame, sobelX, CV_32F, 1, 0, ksize, scale);
        Sobel(frame, sobelY, CV_32F, 0, 1, ksize, scale);

        if (scale == scales[0]) { // 避免浮点比较，改用索引
            sobelX.copyTo(sobelX_total);
            sobelY.copyTo(sobelY_total);
        }
        else {
            sobelX_total += sobelX;
            sobelY_total += sobelY;
        }

            Mat mag;
            magnitude(sobelX, sobelY, mag);
            Mat edges_scale;
            convertScaleAbs(mag, edges_scale);
            edges += edges_scale;
        }

        Mat angles;
        phase(sobelX_total, sobelY_total, angles, true);
        Mat angle_mask;
        inRange(angles, 0, 360, angle_mask);

        Mat edges_thresh;
        adaptiveThreshold(edges, edges_thresh, 255, ADAPTIVE_THRESH_GAUSSIAN_C,
            THRESH_BINARY, 11, 2);
        bitwise_and(edges_thresh, angle_mask, edges_thresh);

        return edges_thresh;
}

Mat FoamDetector::detectDynamicChanges(Mat prev_frame, Mat current_frame) {
    // 实现动态变化检测逻辑
    Mat frame_diff;
    absdiff(prev_frame, current_frame, frame_diff);
    return frame_diff;
}

vector<Rect> FoamDetector::smartMorphology(Mat binary_img) {
		// 实现智能形态学处理逻辑
		int h = binary_img.rows;
        int w = binary_img.cols;
        int min_dim = min(h, w);
        int kernel_size = max(3, min_dim / 100);
        Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(kernel_size, kernel_size));

        Mat morph;
        morphologyEx(binary_img, morph, MORPH_OPEN, kernel, Point(-1, -1), 2);
        morphologyEx(morph, morph, MORPH_CLOSE, kernel, Point(-1, -1), 2);

        Mat labels, stats, centroids;
        int num_labels = connectedComponentsWithStats(morph, labels, stats, centroids, 8);

        vector<Rect> valid_regions;
        int min_area = 100;
        for (int i = 1; i < num_labels; ++i) {
            int area = stats.at<int>(i, CC_STAT_AREA);
            if (area >= min_area) {
                int x = stats.at<int>(i, CC_STAT_LEFT);
                int y = stats.at<int>(i, CC_STAT_TOP);
                int width = stats.at<int>(i, CC_STAT_WIDTH);
                int height = stats.at<int>(i, CC_STAT_HEIGHT);
                valid_regions.emplace_back(x, y, width, height);
            }
        }

        return valid_regions;
}

int FoamDetector::processFrame(Mat frame, Mat& prev_frame, Mat& result_frame) {
    Mat processed_frame = preprocessFrame(frame);
    Mat edges = detectEdges(processed_frame);
    vector<Rect> valid_regions = smartMorphology(edges);

    int foam_count = valid_regions.size();

    if (!prev_frame.empty()) {
        // 检查 prev_frame 和 current_frame 的大小和通道数
        cout << "prev_frame size: " << prev_frame.size() << ", channels: " << prev_frame.channels() << endl;
        cout << "current_frame size: " << frame.size() << ", channels: " << frame.channels() << endl;

        Mat current_processed = preprocessFrame(frame);
        Mat frame_diff = detectDynamicChanges(prev_frame, current_processed);
        Mat thresh;
        threshold(frame_diff, thresh, 15, 255, THRESH_BINARY);

        vector<vector<Point>> contours;
        findContours(thresh, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        for (const auto& cnt : contours) {
            if (contourArea(cnt) > 100) {
                foam_count++;
            }
        }
    }

    putText(frame, "Foam: " + to_string(foam_count), Point(10, 30),
        FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

    if (foam_count > alert_threshold) {
        cout << "Foam count exceeds threshold!" << endl;
        ofstream fout("foam_alert.txt", ios::app);
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        fout << "警报时间: " << ctime(&now);
    }

    result_frame = frame;
    return foam_count;
}

//int main() {
//    FoamDetector detector;
//    detector.setAlertThreshold(100);
//    detector.setPlaybackSpeed(1.0);
//
//    detector.processVideo(“5.mp4”);
//
//    return 0;
//}




#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <random>
#include <stdlib.h>


std::string generate(size_t size) {

    std::vector<char> ans(size);
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto &x: ans) {
        x = static_cast<char>(dist(rd));
    }
    std::string s(ans.begin(), ans.end());
    return s;
//    return "hellohellohellohellohellohellohe";
}


void testing_with_answer() {
    size_t batch_size = 32;
    cv::QRCodeEncoder::Params params;
    cv::Ptr<cv::QRCodeEncoder> encoder = cv::QRCodeEncoder::create(params);
    cv::QRCodeDetector detector;
    const int fps = 30;

    cv::Mat frame, bbox;
    cv::VideoCapture vid(0, cv::CAP_DSHOW);
    vid.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    vid.set(cv::CAP_PROP_FRAME_WIDTH, 1280);

    bool success = true;
    std::string data;

    cv::Mat qr_code, qr_code_resized;

    while (true) {
        if (success) {
            data = generate(batch_size);
            encoder->encode(data, qr_code);
            cv::resize(qr_code, qr_code_resized, cv::Size(), 20, 20, cv::INTER_AREA);
            cv::imshow("Test", qr_code_resized);
        }
        if (cv::waitKey(1000 / fps) >= 0) {
            break;
        }
        vid.read(frame);
        cv::imshow("Camera", frame);
        if (cv::waitKey(1000 / fps) >= 0) {
            break;
        }
        bool flag = false;
        while (!detector.detect(frame, bbox)) {
            _sleep(100);
            vid.read(frame);
            if (cv::waitKey(1000 / fps) >= 0) {
                flag = true;
                break;
            }
        }
        if (flag) {
            break;
        }
        std::string s = detector.decode(frame, bbox);
        if (s == "") {
            success = false;
            continue;
        }
        std::cout << "s = " << s << std::endl << "data = " << data << std::endl;

        success = (s == data);
    }

}


void detection_test() {
    size_t batch_size = 32;
    cv::QRCodeDetector detector;
    cv::QRCodeEncoder::Params params;
    cv::Ptr<cv::QRCodeEncoder> encoder = cv::QRCodeEncoder::create(params);
    const int fps = 30;

    cv::VideoCapture vid(0, cv::CAP_DSHOW);
    vid.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    vid.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    std::string data = generate(batch_size);
    cv::Mat qrcode, frame, bbox;
    encoder->encode(data, qrcode);
    cv::resize(qrcode, qrcode, cv::Size(), 20, 20, cv::INTER_AREA);
    cv::imshow("QR", qrcode);
    while (true) {
        vid.read(frame);
        if (detector.detect(frame, bbox)) {
            cv::Point2i p1(bbox.at<float>(0, 1), bbox.at<float>(0, 3)), p2(bbox.at<float>(0, 2), bbox.at<float>(0, 4));
            std::cout << "Detected " << p1 << ' ' << p2 << std::endl;

            cv::rectangle(frame, p1,
                          p2, cv::Scalar(255, 0, 0), 3);
        }
        cv::imshow("Camera", frame);
        if (cv::waitKey(1000 / fps) >= 0) {
            break;
        }
    }
}

int main() {
//   std::string input_info = "Some text to test, more or less big, i tried differents things";
//   cv::QRCodeEncoder::Params params;
//    params.structure_number = 3;
//    params.mode = cv::QRCodeEncoder::EncodeMode::MODE_STRUCTURED_APPEND;
//
//    cv::Ptr<cv::QRCodeEncoder> encoder = cv::QRCodeEncoder::create(params);
//
//    std::vector<cv::Mat> qrcodes;
//
//  //  encoder->encodeStructuredAppend(input_info, qrcodes);
//
//    for (size_t k = 0; k < qrcodes.size(); k++) {
//        cv::Mat qrcode = qrcodes[k];
//        std::string sav = std::to_string(k) + ".jpg";
//        cv::imwrite(sav, qrcode);
//        std::cout << "Path :" << sav << std::endl;
//    }
//
//    cv::QRCodeDetector detector;
//    cv::Mat imggg = cv::imread("333.jpg");
//    cv::Mat bbox, rectifiedImage;
//    bool is_detected = detector.detect(imggg, bbox);
//    std::vector<char> output;
//    std::string s = detector.decodeCurved(imggg, bbox, output);
//    std::cout << is_detected << '\n';
//    std::cout << bbox << '\n';
//    std::cout << output.size() << "\n";
//    cv::Mat frame;
//
//    std::cout << "Opening" << std::endl;
//    cv::VideoCapture vid(0, cv::CAP_DSHOW);
//    vid.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
//    vid.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
//    std::cout << "Opened" << std::endl;
//    const int fps = 30;
//    if (!vid.isOpened()) {
//        return -1;
//    }
//
//    while (vid.read(frame)) {
//        //std::cout << frame.size << std::endl;
//        cv::imshow("Webcam", frame);
//
//        if (cv::waitKey(1000/fps) >= 0) {
//            break;
//        }
//    }
    //testing_with_answer();
    detection_test();
    return 0;
}

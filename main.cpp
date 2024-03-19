#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <random>
#include <stdlib.h>


struct Transmitter {
    bool is_fixed = true;
    size_t fixed_gererator_pos = 0;
    size_t batch_size = 30;
    int spf = 5;
    std::string fixed_text = "Yesterday\n"
                             "All my troubles seemed so far away,\n"
                             "Now it looks as though they’re here to stay.\n"
                             "Oh, I believe in yesterday.\n"
                             "\n"
                             "Suddenly\n"
                             "I’m not half the man I used to be,\n"
                             "There’s a shadow hanging over me.\n"
                             "Oh, yesterday came suddenly.\n"
                             "\n"
                             "Why she\n"
                             "Had to go, I don’t know, she wouldn’t say.\n"
                             "I said\n"
                             "Something wrong, now I long for yesterday.\n"
                             "\n"
                             "Yesterday\n"
                             "Love was such an easy game to play.\n"
                             "Now I need a place to hide away.\n"
                             "Oh, I believe in yesterday.\n"
                             "\n"
                             "Why she\n"
                             "Had to go, I don’t know, she wouldn’t say.\n"
                             "I said\n"
                             "Something wrong, now I long for yesterday.\n"
                             "\n"
                             "Yesterday\n"
                             "Love was such an easy game to play.\n"
                             "Now I need a place to hide away.\n"
                             "Oh, I believe in yesterday.";


    static std::string generate_random(size_t size) {

        std::vector<char> ans(size);
        std::random_device rd;
        std::uniform_int_distribution<int> dist(0, 255);
        for (auto &x: ans) {
            x = static_cast<char>(dist(rd));
        }
        std::string s(ans.begin(), ans.end());
        return s;
    }

    std::string generate_fixed(size_t size) {
        fixed_gererator_pos += size;
        return fixed_text.substr(fixed_gererator_pos - size, size);
    }


    std::string get_data(size_t size) {
        if (is_fixed) {
            return generate_fixed(size);
        }
        return generate_random(size);
    }

    void sent() {
        cv::QRCodeEncoder::Params params;
        cv::Ptr<cv::QRCodeEncoder> encoder = cv::QRCodeEncoder::create(params);
        cv::Mat qr_code, qr_code_resized;
        std::string data = get_data(batch_size);
        if (data.empty()) {
            return;
        }
        encoder->encode(data, qr_code);
        cv::resize(qr_code, qr_code_resized, cv::Size(), 20, 20, cv::INTER_AREA);
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        while (true) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() > 1000 * spf) {
                begin = std::chrono::steady_clock::now();
                data = get_data(batch_size);
                if (data.empty()) {
                    break;
                }
                encoder->encode(data, qr_code);
                cv::resize(qr_code, qr_code_resized, cv::Size(), 20, 20, cv::INTER_AREA);
            }
            cv::imshow("Transmitter", qr_code_resized);
            cv::waitKey(1);
            _sleep(1000 / 15);
            end = std::chrono::steady_clock::now();
        }
    }
};

struct Receiver {
    int delay_to_start = 10000;
    int spf = 15;


    std::string receive() {
        cv::QRCodeDetector detector;
        cv::Mat frame, bbox;
        cv::VideoCapture vid(0, cv::CAP_DSHOW);
        vid.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
        vid.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        for (int i = 0; i < 1000; i++) {
            vid.read(frame);
            cv::imshow("Receiver", frame);
            if (cv::waitKey(10) >= 0) {
                break;
            }
        }

        std::stringstream result;
        while (true) {
            std::unordered_map<std::string, int> received = {{"", 0}};
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::string max_string;
            int max_string_count;

            while (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() < 5000) {

                vid.read(frame);

                if (!detector.detect(frame, bbox)) {
                    received[""] += 1;
                    cv::imshow("Receiver", frame);
                    end = std::chrono::steady_clock::now();
                    cv::waitKey(1);
                    _sleep(1000 / 15);
                    continue;
                }

                cv::Point2i p1(bbox.at<float>(0, 1), bbox.at<float>(0, 3)), p2(bbox.at<float>(0, 2),
                                                                               bbox.at<float>(0, 4));

                cv::Mat frame_detected(frame);
                cv::rectangle(frame_detected, p1,
                              p2, cv::Scalar(255, 0, 0), 3);
                std::string s = detector.decode(frame, bbox);
                std::cout << s << std::endl;
                cv::imshow("Receiver", frame_detected);
                if (received.find(s) != received.end()) {
                    received[s] += 1;
                } else {
                    received[s] = 1;
                }
                if (max_string_count < received[s]) {
                    max_string_count = received[s];
                    max_string = s;
                }
                cv::waitKey(1);
                _sleep(1000 / 15);
                end = std::chrono::steady_clock::now();
            }
//            if (max_string.empty()) {
//                break;
//            }
            std::cout << max_string << '\n';
            result << max_string;
        }
        return result.str();
    }
};


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
            data = Transmitter::generate_random(batch_size);
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
    std::string data = Transmitter::generate_random(batch_size);
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
    Transmitter t;
    Receiver r;
     std::thread thread1([&t](){_sleep(10000); t.sent();});
    std::thread thread2([&r]() { r.receive(); });
    thread1.join();
    thread2.join();
    return 0;
}

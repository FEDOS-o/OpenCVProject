#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <random>
#include <stdlib.h>
#include <unordered_set>


struct Transmitter {
    size_t fixed_gererator_pos = 0;
    size_t batch_size = 30;
    int fps = 5;
    std::string fixed_text = "Yesterday\n"
                             "All my troubles seemed so far away,\n"
                             "Now it looks as though they're here to stay.\n"
                             "Oh, I believe in yesterday.\n"
                             "\n"
                             "Suddenly\n"
                             "I’m not half the man I used to be,\n"
                             "There's a shadow hanging over me.\n"
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


    std::string get_data(size_t size) {
        if (fixed_gererator_pos >= fixed_text.size()) {
            fixed_gererator_pos = 0;
        }
        std::string res;
        union {
            unsigned int integer;
            char byte[4];
            int integer2;
        } foo;
        foo.integer = fixed_gererator_pos / size;
        for (int i = 0; i < 4; i++) {
            res.push_back(foo.byte[i]);
        }
        foo.integer = (fixed_text.size() + size - 1) / size;
        for (int i = 0; i < 4; i++) {
            res.push_back(foo.byte[i]);
        }
        res += fixed_text.substr(fixed_gererator_pos, size);
        fixed_gererator_pos += size;
        return res;
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
            if (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() > 1000 / fps) {
                begin = std::chrono::steady_clock::now();
                data = get_data(batch_size);
                encoder->encode(data, qr_code);
                cv::resize(qr_code, qr_code_resized, cv::Size(), 20, 20, cv::INTER_AREA);
            }
            cv::imshow("Transmitter", qr_code_resized);
            cv::waitKey(1);
            _sleep(1000 / fps / 10);
            end = std::chrono::steady_clock::now();
        }
    }
};

struct Receiver {
    std::string receive() {
        cv::QRCodeDetector detector;
        cv::Mat frame, bbox;
        cv::VideoCapture vid(0, cv::CAP_DSHOW);
        vid.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
        vid.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        std::unordered_map<int, std::string> stat;
        std::stringstream result;
        while (true) {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            vid.read(frame);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Reading frame" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
            begin = std::chrono::steady_clock::now();
            if (!detector.detect(frame, bbox)) {
                end = std::chrono::steady_clock::now();
                std::cout << "Failure detect" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
                continue;
            }
            end = std::chrono::steady_clock::now();
            std::cout << "Success detect" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
            //cv::Point2i p1(bbox.at<float>(0, 1), bbox.at<float>(0, 3)), p2(bbox.at<float>(0, 2), bbox.at<float>(0, 4));
           // cv::Mat frame_detected(frame);
           // cv::rectangle(frame_detected, p1, p2, cv::Scalar(255, 0, 0), 3);
            begin = std::chrono::steady_clock::now();
            std::string s = detector.decodeCurved(frame, bbox);
            if (s.empty()) {
                continue;
            }
            union {
                int num;
                char bytes[4];
            } foo;
            for (int  i = 0 ; i < 4; i++) {
                foo.bytes[i] = s[i];
            }
            std::cout << "Received: " << foo.num << "/";
            if (stat.find(foo.num) != stat.end()) {
                if (stat[foo.num] != s.substr(8)) {
                    throw std::exception();
                }
            } else {
                stat[foo.num] = s.substr(8);
                std::cout << "New size: " << stat.size() << std::endl;
            }

            for (int  i = 4 ; i < 8; i++) {
                foo.bytes[i - 4] = s[i];
            }
            std::cout << foo.num << std::endl;
            if (stat.size() == foo.num) {
                break;
            }
            end = std::chrono::steady_clock::now();
            std::cout << "Decode only" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
        }
        std::vector<std::pair<int, std::string>> stat2(stat.begin(), stat.end());
        std::sort(stat2.begin(), stat2.end());
        for (auto [key, value] : stat2) {
            result << value;
        }
        std::cout << result.str() << std::endl;
        return result.str();
    }
};


int main() {
    Transmitter t;
    Receiver r;
    std::thread thread1([&t]() {
        t.sent();
    });
    std::thread thread2([&r]() { r.receive(); });
    thread1.join();
    thread2.join();
    return 0;
}

#include"stick_figure_detect.h"
#include<iostream>
#include<string>
#include<opencv2/imgcodecs.hpp>

using namespace std;

int main(int argc, char** argv){
    if(argc < 2){
        cout<<"param error!"<<endl;
        cout<<"please input: stick_detect filename1 [filename2 filename3 ...]"<<endl;
        return 0;
    }
    else{
        for(int i = 1;i < argc ;i++){
            cv::Mat src = cv::imread(argv[i]);
            StickFigure stick;
            auto res = stick.is_stick_figure(src, 20);
            cv::Mat binary_mat = stick.to_binary_stick(20);
            cv::imwrite(argv[i] + string("_gray.jpg"),binary_mat);
            auto stick_compress = stick.compress();
            stick_compress.save(argv[i] + string("_compress.sc"));
            StickCompress sc_reader(argv[i] + string("_compress.sc"));
            cv::Mat binary_mat2 = sc_reader.to_cv_mat();
            cv::imwrite(argv[i] + string("_gray2.jpg"),binary_mat2);
            
            cout<<(res?"True ":"False ");
        }
        cout<<endl;
    }
    return 0;
}

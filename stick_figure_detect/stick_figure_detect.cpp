#include "stick_figure_detect.h"
#include <iostream>
#include <fstream>
#include <string>
#include<unordered_set>
#define PACK_POS(x,y) (x<<8| y)
#define UNPACK_X(pos) (pos>>8)
#define UNPACK_Y(pos) (pos&0xFF)

const unsigned int head_bit = 0xF219A011;
using namespace std;
StickCompress::StickCompress(){
    reset();
}
StickCompress::StickCompress(std::string file_name){
    reset();

    #define RIFS(buf) ifs.read((char*)&buf, sizeof(buf));
    ifstream ifs(file_name, ios::binary|ios::in);
    if(ifs.is_open()){
        unsigned int head = 0;
        RIFS(head);
        if(head_bit != head){
            ifs.close();
            return;
        }
        uchar line_count = 0;
        RIFS(img_width);
        RIFS(img_height);
        RIFS(line_count);
        for (int i = 0;i < line_count;i++){
            uchar row = 0;
            uchar range_count = 0;
            RIFS(row);
            RIFS(range_count);
            std::vector<uchar> cols_range(range_count);
            ifs.read((char*)&cols_range[0], range_count);
            stick_point_compress_[row] = cols_range;
        }
    }
    ifs.close();
    for(auto line:stick_point_compress_){
        for(size_t i = 0;i < line.second.size();i+=2){
            for(uchar j = line.second[i]; j < line.second[i+1];j++){
                stick_point_.push_back({j,line.first});
            }
        }

    }
}

void StickCompress::reset(){
    offset = {0,0};
    stick_point_.clear();  
    stick_point_compress_.clear();
    box_ = {9999, 9999, 0, 0}; //letf, top, right, bottom
    img_width = 0;
    img_height = 0;
}
void StickCompress::push_stick(int x, int y){
    stick_point_.push_back({x, y});
    box_[0] > x ? box_[0] = x:box_[0];
    box_[2] < x ? box_[2] = x:box_[0];
    box_[1] > y ? box_[1] = y:box_[0];
    box_[3] < y ? box_[3] = y:box_[0];
}
bool StickCompress::scale_to_size(int  width ,int height){
    unordered_set<uint16_t> stick_point_map;
    if(width > 256 || height > 256){
        return false; 
    }
    img_width = width;
    img_height  = height;
    int src_width = box_[2] - box_[0];
    int src_height = box_[3] - box_[1];
    float target_wh_rate  = width / (float)height;
    float src_wh_rate = src_width/float(src_height);
    float scale = 1.0;
    if (src_wh_rate > target_wh_rate){
        scale = width / float(src_width + 1);
        offset.y = (height - src_height*scale)/2;
    }
    else{
        scale = height / float(src_height +1);
        offset.x = (width - src_width*scale)/2;
    }
    for(auto tp : stick_point_){
        stick_point_map.insert(PACK_POS((uint16_t)((tp.x - box_[0])* scale + offset.x),(uint16_t)((tp.y - box_[1]) * scale + offset.y)));
    }

    for(auto p :stick_point_map){
        stick_point_compress_[UNPACK_Y(p)].push_back(UNPACK_X(p));
    }
    for(auto& line : stick_point_compress_){
        uchar start = line.second[0];
        uchar end = line.second[0] + 1;
        std::vector<uchar> compress_res;
        for(size_t i = 1;i < line.second.size();i++){
            if(end == line.second[i]){
                end = line.second[i] + 1;
            }
            else{
                compress_res.push_back(start);
                compress_res.push_back(end);
                start = line.second[i];
                end = start+1;
            }
        }
        compress_res.push_back(start);
        compress_res.push_back(end);
        line.second.swap(compress_res);
    }

}
    

std::vector<char> StickCompress::to_buf(){
    if(stick_point_compress_.empty()){
       scale_to_size(256,196); 
       std::cout<<"did not scale img ,defualt scale to 256*106"<<std::endl;
    }
    size_t compress_size = 0;
    for(auto line: stick_point_compress_){
        compress_size +=2;
        compress_size +=line.second.size() ;
    }
    int buf_size = sizeof(head_bit) + sizeof(img_width)*2  + sizeof(int) +compress_size;
    std::vector<char> buf(buf_size);
    char* cur_ptr = &buf[0]; 
    uchar line_count = stick_point_compress_.size();
#define WRITE2BUF(data) memcpy(cur_ptr, &data, sizeof(data));cur_ptr += sizeof(data);
    WRITE2BUF(head_bit);
    WRITE2BUF(img_width);
    WRITE2BUF(img_height);
    WRITE2BUF(line_count);
    for(auto line: stick_point_compress_){
        uchar range_count = line.second.size();
        WRITE2BUF(line.first);
        WRITE2BUF(range_count);
        memcpy(cur_ptr, &line.second[0], line.second.size());
        cur_ptr += line.second.size();
    }

    return buf;
}

cv::Mat StickCompress::to_cv_mat(){
    cv::Mat res = cv::Mat::zeros(img_height, img_width,  CV_8UC1);
    for(auto p: stick_point_){
        res.at<uchar>(p.y,p.x) = 255;
    }
    return res;
}

void StickCompress::save(std::string file_name){
    std::ofstream ofs(file_name, ios::binary| ios::out);
    auto buf = to_buf();
    ofs.write(buf.data(), buf.size());
    ofs.close();
}

class StickFigure::StickFigureImpl{
public:

    cv::Mat src;
    cv::Mat hist[3]; 
    StickCompress stick_compress;

    StickFigureImpl(){};
    ~StickFigureImpl(){};
    
    void reset(){
        hist[0] = cv::Mat::zeros(1, 256, CV_32SC1);
        hist[1] = cv::Mat::zeros(1, 256, CV_32SC1);
        hist[2] = cv::Mat::zeros(1, 256, CV_32SC1);
        
        stick_compress.reset();
        
    }

    bool is_stick_figure(cv::Mat src_in, int threshold){
        src = src_in;
        reset();
        if(src.type() == CV_8UC1){
            int black_count = 0;
    
            for(int r = 0; r < src.rows;r++){
                for(int c = 0; c < src.cols;c++){
                    if(src.at<uchar>(r,c) < 128){
                        black_count ++;
                    }
               }
            }
            return black_count < src.rows* src.cols /4;
        }
        else if( src.type() == CV_8UC3 || src.type() == CV_8UC4){
            int black_cout = 0;
            int white_cout = 0;
            
            for(int r = 0; r < src.rows;r++){
                for(int c = 0; c < src.cols;c++){
                    uchar* ptr =  src.ptr(r,c); 
                    hist[0].at<int>(ptr[0])++;
                    hist[1].at<int>(ptr[1])++;
                    hist[2].at<int>(ptr[2])++;
                    if(ptr[0] < threshold && ptr[1] < threshold  && ptr[2] < threshold){
                        black_cout ++;
                    }
                    else if(ptr[0] > 255 -threshold && ptr[0] > 255 -threshold  && ptr[1] > 255 -threshold){
                        white_cout ++;
                    }
                }
            }
            if((black_cout + white_cout)/(float)(src.rows*src.cols) > 0.9 && black_cout /(float)(src.rows*src.cols) < 0.25){
                return true;
            }
        }
    
        return false;
    }

};

StickFigure::StickFigure():impl_(std::make_unique<StickFigureImpl>()){

}
StickFigure::~StickFigure(){
}
bool StickFigure::is_stick_figure(cv::Mat src, int threshold){
    return impl_->is_stick_figure(src, threshold);
}

cv::Mat StickFigure::to_binary_stick(int threshold){
    auto src = impl_->src;
    cv::Mat res = cv::Mat::zeros(src.size(),CV_8UC1);
        for(int r = 0; r < src.rows;r++){
            for(int c = 0; c < src.cols;c++){
                uchar* ptr =  src.ptr(r,c); 
                if(ptr[0] < threshold && ptr[1] < threshold && ptr[2] < threshold
                   && abs(ptr[0] - ptr[1]) + abs(ptr[1] - ptr[2]) < threshold){
                    res.at<uchar>(r,c) = 255;
                    impl_->stick_compress.push_stick(c,r);
                }
            }
        }
 
    return res;
}


StickCompress& StickFigure::compress(){
    return impl_->stick_compress;
}
